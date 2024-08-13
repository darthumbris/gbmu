#include "Cpu.hpp"
#include "Interruptor.hpp"
#include "OpcodeTiming.hpp"
#include <cstdint>
#include <ctime>

uint64_t const DEBUG_START = 0;
uint64_t const DEBUG_COUNT = 1;

Cpu::Cpu(Decoder dec, const std::string path)
    : decoder(dec), mmap(path, this), ppu(this), rom_path(path), interruptor(this) {
	u8_registers = {0};
	pc = 0;
	sp = 0;
	debug_count = 0;
	m_cycle = 0;
	t_cycle = 0;
	mmap.init_memory();
	ppu.init_hdma();
	ppu.init_ppu_mem();
	set_instructions();
}

Cpu::~Cpu() {}

uint16_t Cpu::get_16bitregister(Registers reg) const {
	if (reg == Registers::SP) {
		return sp;
	}
	return ((uint16_t)(u8_registers[reg - Registers::BC] << 8) + (uint16_t)u8_registers[reg - Registers::BC + 1]);
}

uint8_t Cpu::get_register(Registers reg) const {
	return (u8_registers[reg]);
}

uint8_t Cpu::get_flag(uint8_t flag) const {
	return ((u8_registers[Registers::F] >> flag) & 1);
}

void Cpu::set_16bitregister(Registers reg, uint16_t val) {
	if (reg == Registers::SP) {
		sp = val;
	} else {
		u8_registers[reg - Registers::BC] = (uint8_t)(val >> 8);
		u8_registers[reg - Registers::BC + 1] = (uint8_t)(val & 0xff);
	}
}

void Cpu::set_register(Registers reg, uint8_t val) {
	u8_registers[reg] = val;
}

void Cpu::set_flag(uint8_t flag, uint8_t val) {
	u8_registers[Registers::F] ^= ((-val) ^ u8_registers[Registers::F]) & (1U << flag);
}

void Cpu::tick() {
	if (!locked) {
		while (ppu.screen_ready())
			;
#ifdef DEBUG_MODE
// std::cout << debug_count << " opcode: 0x" << std::hex
//             << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << std::endl;
#endif
		execute_instruction();
		uint16_t t_cycle_u8 = (uint8_t)t_cycle;
		interruptor.timer_tick(t_cycle_u8);
		interruptor.serial_tick(t_cycle_u8);
		ppu.tick(t_cycle_u8);
		interruptor.input_tick(t_cycle_u8);
		m_cycle = 0;
		t_cycle = 0;
	}
	event_handler();
	debug_count += 1;
}

uint16_t Cpu::cycle_speed(uint16_t cycle) {
	if (cycle == 0) {
		return 0;
	}
	return cycle >> speed_multiplier;
}

void Cpu::process_interrupt(InterruptType i) {
	sp -= 2;
	mmap.write_u16(sp, pc);

	if (i == InterruptType::Vblank) {
		pc = 0x40;
	} else if (i == InterruptType::Stat)
		pc = 0x48;
	else if (i == InterruptType::Timer)
		pc = 0x50;
	else if (i == InterruptType::Serial)
		pc = 0x58;
	else if (i == InterruptType::Joypad)
		pc = 0x60;
	set_cycle(5);
}

#ifdef DEBUG_MODE
void Cpu::debug_print(bool prefix) {
	std::cout << "[0x" << std::setfill('0') << std::setw(4) << std::hex << pc;
	std::cout << "] 0x" << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << "\t";

	if (prefix) {
		decoder.prefixed_instructions[opcode].print_instruction();
	} else {
		decoder.instructions[opcode].print_instruction();
	}
}
#endif

void Cpu::prefix() {
	opcode = mmap.read_u8(pc);
#ifdef DEBUG_MODE
	DEBUG_MSG("opcode: 0x%02X\n", opcode);
	// DEBUG_MSG("register AF %u BC %u DE %u HL %u SP %u PC: %u\n", get_16bitregister(Registers::AF),
	// get_16bitregister(Registers::BC), get_16bitregister(Registers::DE), get_16bitregister(Registers::HL), sp, pc);
	if (debug_count > DEBUG_START && debug_count < DEBUG_START + DEBUG_COUNT) {
		// debug_print(true);
		// DEBUG_MSG("debug_count: %lu opcode CB: %#04x pc: %u\n", debug_count, opcode, pc);
		// DEBUG_MSG("register a %u b %u f %u HL %u SP %u\n", u8_registers[Registers::A], u8_registers[Registers::B],
		//        u8_registers[Registers::F], get_16bitregister(Registers::HL), sp);
		// DEBUG_MSG("C %u D %u E %u\n", u8_registers[Registers::C], u8_registers[Registers::D],
		// u8_registers[Registers::E]);
	}
#endif
	pc += 1;
	auto op = prefixed_instructions[opcode];
	(this->*op)();
}

void Cpu::handle_halt() {
	if (accurate_opcode_state == StateReady && halted) {
		set_cycle(1);

		if (halt_cycle > 0) {
			halt_cycle -= t_cycle;

			if (halt_cycle <= 0) {
				halt_cycle = 0;
				halted = false;
			}
		}

		if (halted && interruptor.pending() != NoInterrupt && halt_cycle == 0) {
			halt_cycle = cycle_speed(12);
		}
	}
}

void Cpu::execute_instruction() {
	uint8_t executed_cycles = 0;

	while (executed_cycles < 1) {
		interruptor.reset_interrupt();
		t_cycle = 0;
		m_cycle = 0;
		handle_halt();

#ifdef DEBUG_MODE
		if (!halted) {
			DEBUG_MSG("i %u s %u m %u p %u\n", interruptor.pending(), accurate_opcode_state, interruptor.get_ime(), pc);
		}
#endif
		if (!halted && !interruptor.handle_interrupt(accurate_opcode_state)) {
			const uint8_t *accurateOPcodes = UnprefixedAccurate;
			const uint8_t *machineCycles = UnprefixedMachineCycles;
			OpsFn *opcodeTable = unprefixed_instructions;

			opcode = mmap.read_u8(pc);
			pc += 1;
			bool isCB = (opcode == 0xCB);
#ifdef DEBUG_MODE
			DEBUG_MSG("op 0x%02X state: %u c %u\n", opcode, accurate_opcode_state, t_cycle);
			if (opcode != 0xCB) {
				// if (debug_count > DEBUG_START && debug_count < DEBUG_START + DEBUG_COUNT) {
				// debug_print(false);
				// 	DEBUG_MSG("debug_count: %lu opcode: %#04x pc: %u\n", debug_count, opcode, pc);
				// 	DEBUG_MSG("register a %u b %u f %u HL %u SP %u\n", u8_registers[Registers::A],
				// u8_registers[Registers::B], 	       u8_registers[Registers::F], get_16bitregister(Registers::HL),
				// sp); 	DEBUG_MSG("C %u D %u E %u\n", u8_registers[Registers::C], u8_registers[Registers::D],
				// u8_registers[Registers::E]);
				// }
			}
#endif

			if (isCB) {
				accurateOPcodes = PrefixedAccurate;
				machineCycles = PrefixedMachineCycles;
				opcodeTable = prefixed_instructions;
				opcode = mmap.read_u8(pc);
				DEBUG_MSG("op 0x%02X\n", opcode);
				pc += 1;
			}
			if ((accurateOPcodes[opcode] != 0) && (accurate_opcode_state == StateReady)) {
				int left_cycles = (accurateOPcodes[opcode] < 3 ? 2 : 3);
				set_cycle((machineCycles[opcode] - left_cycles));
				accurate_opcode_state = StateReadingWord;
				pc -= 1;
				if (isCB) {
					pc -= 1;
				}
			} else {
				(this->*opcodeTable[opcode])();
				if (branched) {
					branched = false;
					set_cycle((BranchedMachineCycles[opcode]));
				} else {
					switch (accurate_opcode_state) {
					case StateReady:
						set_cycle((machineCycles[opcode]));
						break;
					case StateReadingWord:
						if (accurateOPcodes[opcode] == 3) {
							set_cycle(1);
							accurate_opcode_state = StateReadingByte;
							pc -= 1;
							if (isCB) {
								pc -= 1;
							}
						} else {
							set_cycle(2);
							accurate_opcode_state = StateReady;
						}
						break;
					case StateReadingByte:
						set_cycle(2);
						accurate_opcode_state = StateReady;
						break;
					}
				}
			}
		}
		interruptor.check_cycles(t_cycle, accurate_opcode_state);
		DEBUG_MSG("AF %u BC %u DE %u HL %u SP %u PC %u i %u c %u\n", get_16bitregister(Registers::AF),
		          get_16bitregister(Registers::BC), get_16bitregister(Registers::DE), get_16bitregister(Registers::HL),
		          sp, pc, mmap.read_u8(0xFF0F), t_cycle);
		executed_cycles += t_cycle;
	}
	// DEBUG_MSG("clockycle: %u speed: %u state: %u\n", executed_cycles, speed_multiplier, accurate_opcode_state);
}
