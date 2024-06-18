#include "Cpu.hpp"
#include <typeinfo>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <ctime>

uint64_t const DEBUG_START = 26979095;
uint64_t const DEBUG_COUNT = 6381650;

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
    // if (reg == Registers::A && val == 254) {
    //     std::cout << "here" << std::endl;
    // }
    u8_registers[reg] = val;
}

INLINE_FN void Cpu::set_flag(uint8_t flag, uint8_t val)
{
    u8_registers[Registers::F] ^= ((-val) ^ u8_registers[Registers::F]) & (1U << flag);
}

void Cpu::tick()
{
    while (ppu.draw_screen)
        ;

    // std::cout << debug_count << " opcode: 0x" << std::hex
    //             << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << std::endl;
    if (opcode != 0xFB && opcode != 0xD9) {
        handle_interrupt();
    }

    execute_instruction();

    ppu.tick(t_cycle);
    printf("cycle: %u\n", t_cycle);
    d_cycle += t_cycle;
    if (d_cycle >= 256) {//TODO check for cpu stopped and handle different frequencies
        timer_divider++;
        d_cycle -= 256;
    }
    // printf("d_cycle: %u timer_divider: %u\n", d_cycle, timer_divider);
    event_handler();
    debug_count += 1;
    m_cycle = 0;
    t_cycle = 0;
}

void Cpu::handle_interrupt()
{
	uint8_t masked = interrupt_enable_register & interrupt;
    // printf("masked: %u, ie: %u, if: %u\n", masked, interrupt_enable_register, interrupt);
	if (!masked)
		return ;
	if (halted)
		halted = false;

	// Handle the interrupt, while taking the priority into account
	if (process_interrupts)
	{
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
    // printf("i: %u, if: %u\n", i, interrupt);
	if (i == InterruptType::Vblank) {
		pc = 0x40;
    }
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
}

void Cpu::prefix() {
    opcode = mmap.read_u8(pc);
    #ifdef DEBUG_MODE
        if (debug_count > DEBUG_START - DEBUG_COUNT && debug_count < DEBUG_START + DEBUG_COUNT) {
            // debug_print(true);
            std::cout << debug_count << " opcode: 0xCB" << std::hex
                << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << std::endl;
            std::cout << debug_count << " PC: 0x" << std::hex
                << std::setfill('0') << std::setw(4) << pc << std::dec << std::endl;
            printf("register a %u b %u f %u HL %u\n", u8_registers[Registers::A], u8_registers[Registers::B], u8_registers[Registers::F], get_16bitregister(Registers::HL));
			
        }
    #endif
    pc += 1;
    auto op = prefixed_instructions[opcode];
    (this->*op)();

}

void Cpu::execute_instruction()
{
    if (halted) {
        set_cycle(1);
        return;
    }
    opcode = mmap.read_u8(pc);
    #ifdef DEBUG_MODE
        if (opcode != 0xCB) {
            if (debug_count > DEBUG_START - DEBUG_COUNT && debug_count < DEBUG_START + DEBUG_COUNT) {
                // debug_print(false);
                std::cout << debug_count << " opcode: 0x" << std::hex
                << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << std::endl;
                std::cout << debug_count << " PC: 0x" << std::hex
                << std::setfill('0') << std::setw(4) << pc << std::dec << std::endl;
                printf("register a %u b %u f %u HL %u\n", u8_registers[Registers::A], u8_registers[Registers::B], u8_registers[Registers::F], get_16bitregister(Registers::HL));
			} 
        }
    #endif
    pc += 1;
    auto op = unprefixed_instructions[opcode];
    (this->*op)();
}

void Cpu::lockup() {}
