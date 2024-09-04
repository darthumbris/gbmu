#include "Cpu.hpp"
#include "Interruptor.hpp"
#include "OpcodeTiming.hpp"
#include <cassert>
#include <cstdint>
#include <ctime>

Cpu::Cpu(Decoder dec, const options options)
    : decoder(dec), mmap(options, this), ppu(this, options.scale), interruptor(this), rom_path(options.path) {
	load_options = options;
	u8_registers = {0};
	pc = 0;
	sp = 0;
	debug_count = 0;
	m_cycle = 0;
	t_cycle = 0;
	mmap.init_memory();
	ppu.init_hdma();
	ppu.init_ppu_mem();
	mmap.load_ram();
	apu.init(mmap.is_cgb_rom());
	set_instructions();
}

Cpu::~Cpu() {}

void Cpu::reset() {
	mmap.reset();
	locked = false;
	paused = false;
	halted = false;
	u8_registers = {0};
	pc = 0;
	sp = 0;
	debug_count = 0;
	m_cycle = 0;
	t_cycle = 0;
	halt_cycle = 0;
	opcode = 0;
	accurate_opcode_state = instruction_state::Ready;
	read_cache = 0;
	branched = false;
	instruction = instruction_list::Unprefixed;
	cgb_speed = false;
	speed_multiplier = 0;
	interruptor.reset();
	ppu.reset(mmap.is_cgb_rom());
	apu.reset(mmap.is_cgb_rom());
}

uint16_t Cpu::get_16bitregister(registers reg) const {
	if (reg == registers::SP) {
		return sp;
	}
	return (static_cast<uint16_t>(u8_registers[reg - registers::BC] << 8) +
	        static_cast<uint16_t>(u8_registers[reg - registers::BC + 1]));
}

uint8_t Cpu::get_register(registers reg) const {
	return (u8_registers[reg]);
}

uint8_t Cpu::get_flag(uint8_t flag) const {
	return ((u8_registers[registers::F] >> flag) & 1);
}

void Cpu::set_16bitregister(registers reg, uint16_t val) {
	if (reg == registers::SP) {
		sp = val;
	} else {
		u8_registers[reg - registers::BC] = static_cast<uint8_t>(val >> 8);
		u8_registers[reg - registers::BC + 1] = static_cast<uint8_t>(val & 0xff);
	}
}

void Cpu::set_register(registers reg, uint8_t val) {
	u8_registers[reg] = val;
}

void Cpu::set_flag(uint8_t flag, uint8_t val) {
	u8_registers[registers::F] ^= ((-val) ^ u8_registers[registers::F]) & (1U << flag);
}

void Cpu::tick() {
	if (!locked && !paused) {
		bool vblank = false;
		uint32_t total_clocks = 0;
		while (!vblank) {
			uint16_t cycles = handle_instruction();
			interruptor.timer_tick(cycles);
			interruptor.serial_tick(cycles);
			ppu.tick(cycles);
			if (ppu.screen_ready()) {
				vblank = true;
			}
			apu.tick(cycles);
			interruptor.input_tick(cycles);

			total_clocks += cycles;

			if (total_clocks > 702240) {
				vblank = true;
			}

			m_cycle = 0;
			t_cycle = 0;
		}
		apu.end_frame();
		apu.write();
		mmap.update_clock();
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

void Cpu::process_interrupt(interrupt_type i) {
	sp -= 2;
	mmap.write_u16(sp, pc);

	if (i == interrupt_type::Vblank) {
		pc = 0x40;
	} else if (i == interrupt_type::Stat)
		pc = 0x48;
	else if (i == interrupt_type::Timer)
		pc = 0x50;
	else if (i == interrupt_type::Serial)
		pc = 0x58;
	else if (i == interrupt_type::Joypad)
		pc = 0x60;
	set_cycle(5);
}

#ifdef DEBUG_MODE
void Cpu::debug_print(bool prefix) {
	DEBUG_MSG("[%#06X] %#04X\t", pc, opcode);
	if (prefix) {
		decoder.prefixed_instructions[opcode].print_instruction();
	} else {
		decoder.instructions[opcode].print_instruction();
	}
}
#endif

void Cpu::prefix() {}

void Cpu::handle_halt() {
	if (accurate_opcode_state == instruction_state::Ready && halted) {
		set_cycle(1);

		if (halt_cycle > 0) {
			halt_cycle -= t_cycle;

			if (halt_cycle <= 0) {
				halt_cycle = 0;
				halted = false;
			}
		}

		if (halted && interruptor.pending() != interrupt_type::NoInterrupt && halt_cycle == 0) {
			halt_cycle = cycle_speed(12);
		}
	}
}

void Cpu::fetch_instruction() {
	bool is_prefixed = false;
	opcode = mmap.read_u8(pc);
	pc += 1;
	instruction = (opcode == 0xCB) ? instruction_list::Prefixed : instruction_list::Unprefixed;
	if (opcode == 0xCB) {
		opcode = mmap.read_u8(pc);
		pc += 1;
		is_prefixed = true;
	}
#ifdef DEBUG_MODE
	debug_print(is_prefixed);
#endif
}

void Cpu::set_cycles_left() {
	int left_cycles = (accurate_cycles[instruction][opcode] < 3 ? 2 : 3);
	set_cycle((machine_cycles[instruction][opcode] - left_cycles));
	accurate_opcode_state = instruction_state::ReadingWord;
	decrement_pc();
}

void Cpu::execute_instruction() {
	(this->*instructions[instruction][opcode])();
	if (branched) {
		branched = false;
		set_cycle((branched_machine_cycles[opcode]));
	} else {
		switch (accurate_opcode_state) {
		case instruction_state::Ready:
			set_cycle((machine_cycles[instruction][opcode]));
			break;
		case instruction_state::ReadingWord:
			if (accurate_cycles[instruction][opcode] == 3) {
				set_cycle(1);
				accurate_opcode_state = instruction_state::ReadingByte;
				decrement_pc();
			} else {
				set_cycle(2);
				accurate_opcode_state = instruction_state::Ready;
			}
			break;
		case instruction_state::ReadingByte:
			set_cycle(2);
			accurate_opcode_state = instruction_state::Ready;
			break;
		}
	}
}

void Cpu::decrement_pc() {
	pc -= 1;
	if (instruction == instruction_list::Prefixed) {
		pc -= 1;
	}
}

uint8_t Cpu::handle_instruction() {
	uint8_t executed_cycles = 0;

	while (executed_cycles < 1) {
		interruptor.reset_interrupt();
		t_cycle = 0;
		m_cycle = 0;
		handle_halt();

		if (!halted && !interruptor.handle_interrupt(accurate_opcode_state)) {
			fetch_instruction();
			if ((accurate_cycles[instruction][opcode] != 0) && (accurate_opcode_state == instruction_state::Ready)) {
				set_cycles_left();
			} else {
				execute_instruction();
			}
		}
		interruptor.check_cycles(t_cycle, accurate_opcode_state);
		executed_cycles += t_cycle;
	}
	return executed_cycles;
}
