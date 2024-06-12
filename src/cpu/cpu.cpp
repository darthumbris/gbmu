#include "Cpu.hpp"
#include <typeinfo>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <ctime>

uint32_t const DEBUG_START = 0;
uint32_t const DEBUG_COUNT = 48187;

Cpu::Cpu(Decoder dec, const std::string path) : decoder(dec), mmap(path, this), ppu(this)
{
    u8_registers = {0};
    pc = 0;
    sp = 0;
    debug_count = 0;
    m_cycle = 0;
    t_cycle = 0;
    set_instructions();
}

Cpu::~Cpu()
{
}

INLINE_FN uint16_t Cpu::get_16bitregister(Registers reg) const
{
    if (reg == Registers::SP) {
        return sp;
    }
    return ((uint16_t)(u8_registers[reg - Registers::BC] << 8) + (uint16_t)u8_registers[reg - Registers::BC + 1]);
}

INLINE_FN uint8_t Cpu::get_register(Registers reg) const
{
    return (u8_registers[reg]);
}

INLINE_FN uint8_t Cpu::get_flag(uint8_t flag) const
{
    return ((u8_registers[Registers::F] >> flag) & 1);
}

INLINE_FN void Cpu::set_16bitregister(Registers reg, uint16_t val)
{
    if (reg == Registers::SP) {
        sp = val;
    }
    else {
        u8_registers[reg - Registers::BC] = (uint8_t)(val >> 8);
        u8_registers[reg - Registers::BC + 1] = (uint8_t)(val & 0xff);
    }
}

INLINE_FN void Cpu::set_register(Registers reg, uint8_t val)
{
    u8_registers[reg] = val;
}

INLINE_FN void Cpu::set_flag(uint8_t flag, uint8_t val)
{
    u8_registers[Registers::F] ^= ((-val) ^ u8_registers[Registers::F]) & (1U << flag);
}

void Cpu::tick()
{
    // const std::chrono::time_point<std::chrono::steady_clock> start =
    //     std::chrono::steady_clock::now();
    // if (debug_count >= DEBUG_START && debug_count < DEBUG_START + DEBUG_COUNT) {
        // std::cout << debug_count << "  ";
        // std::cout << "debug count: " << debug_count << std::endl;
        // if (debug_count > 2218245 - 2 && debug_count < 2218245 + 2)
        //     debug_print(std::get<0>(dec), std::get<1>(dec));
        // printf("%d, registers b: %u, c: %u, d: %u, e: %u, h: %u, l: %u, a: %u, f: %u\n", debug_count, get_register(Registers::B), get_register(Registers::C), get_register(Registers::D), get_register(Registers::E), get_register(Registers::H), get_register(Registers::L), get_register(Registers::A), get_register(Registers::F));
    // }
    // if (debug_count > 24578 && debug_count < 24630) {
        // std::bitset<8> x(get_register(Registers::F));
        // printf("%d, registers b: %u, c: %u, d: %u, e: %u, h: %u, l: %u, a: %u, f: %u\n", debug_count, get_register(Registers::B), get_register(Registers::C), get_register(Registers::D), get_register(Registers::E), get_register(Registers::H), get_register(Registers::L), get_register(Registers::A), get_register(Registers::F));
        // std::cout << "debug count: "  << debug_count << ", register: " << x << std::endl;
    // }

    if (opcode != 0xFB && opcode != 0xD9) {
        handle_interrupt();
    }

    if (halted) {
        return;
    }

    execute_instruction();
    // if (pc == 0x0100)
    // {
        // std::cout << "Start of ROM" << std::endl;
        // mmap.bios_loaded = true;
        // exit(1);
    // }
    ppu.tick(t_cycle);
    event_handler();
    debug_count += 1;
    m_cycle = 0;
    t_cycle = 0;
    // const auto end = std::chrono::steady_clock::now();
    //  std::cout
    //     << "delta time: "
    //     << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start) << "\n";  // using milliseconds and seconds accordingly
}

void Cpu::handle_interrupt()
{
	uint8_t masked = interrupt_enable_register & interrupt;
	if (!masked)
		return ;
	if (halted)
		halted = false;

	// Handle the interrupt, while taking the priority into account
	if (process_interrupts)
	{
		// Maybe use for loop
		if (masked & InterruptType::Vblank)
			process_interrupt(InterruptType::Vblank);
		else if (masked & InterruptType::Stat)
			process_interrupt(InterruptType::Stat);
		else if (masked & InterruptType::Timer)
			process_interrupt(InterruptType::Timer);
		else if (masked & InterruptType::Serial)
			process_interrupt(InterruptType::Serial);
		else if (masked & InterruptType::Joypad)
			process_interrupt(InterruptType::Joypad);
	}
}

void Cpu::process_interrupt(InterruptType i)
{
	nop();
	nop();

    sp -= 2;
    mmap.write_u16(sp, pc);

	process_interrupts = false;
	interrupt &= ~i;
	if (i == InterruptType::Vblank)
		pc = 0x40;
	else if (i == InterruptType::Stat)
		pc = 0x48;
	else if (i == InterruptType::Timer)
		pc = 0x50;
	else if (i == InterruptType::Serial)
		pc = 0x58;
	else if (i == InterruptType::Joypad)
		pc = 0x60;
    set_cycle(1);
}

void Cpu::debug_print(bool prefix)
{
    std::cout << "[0x" << std::setfill('0') << std::setw(4) << std::hex << pc;
    std::cout << "] 0x" << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << "\t";

    if (prefix) {
        decoder.prefixed_instructions[opcode].print_instruction();
    }
    else {
        decoder.instructions[opcode].print_instruction();
    }

    // if (interrupts)
    //     std::cout << "interrupts: 0x" << std::setfill('0') << std::setw(4) << std::hex << interrupts << std::dec << std::endl;
}

void Cpu::prefix() {
    opcode = mmap.read_u8(pc);
    pc += 1;
    auto op = prefixed_instructions[opcode];
    (this->*op)();

    // #ifdef DEBUG_MODE
        // debug_print(opcode, true);
    // #endif
}

void Cpu::execute_instruction()
{
    opcode = mmap.read_u8(pc);
    pc += 1;
    auto op = unprefixed_instructions[opcode];
    (this->*op)();
    // #ifdef DEBUG_MODE
    //     if (opcode != 0xCB)
    //         debug_print(opcode, false);
    // #endif
}

void Cpu::lockup() {}
