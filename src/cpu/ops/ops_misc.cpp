#include "Cpu.hpp"
#include <iostream>

void Cpu::swap_r8(uint16_t opcode)
{
    uint16_t val;
    switch (opcode)
    {
    case 0x30:
        val = get_register(Registers::B);
        set_register(Registers::B, (val & 0xF) << 4);
        set_register(Registers::B, get_register(Registers::B) | ((val & 0xF0) >> 4));
        break;
    case 0x31:
        val = get_register(Registers::C);
        set_register(Registers::C, (val & 0xF) << 4);
        set_register(Registers::C, get_register(Registers::C) | ((val & 0xF0) >> 4));
        break;
    case 0x32:
        val = get_register(Registers::D);
        set_register(Registers::D, (val & 0xF) << 4);
        set_register(Registers::D, get_register(Registers::D) | ((val & 0xF0) >> 4));
        break;
    case 0x33:
        val = get_register(Registers::E);
        set_register(Registers::E, (val & 0xF) << 4);
        set_register(Registers::E, get_register(Registers::E) | ((val & 0xF0) >> 4));
        break;
    case 0x34:
        val = get_register(Registers::H);
        set_register(Registers::H, (val & 0xF) << 4);
        set_register(Registers::H, get_register(Registers::H) | ((val & 0xF0) >> 4));
        break;
    case 0x35:
        val = get_register(Registers::L);
        set_register(Registers::L, (val & 0xF) << 4);
        set_register(Registers::L, get_register(Registers::L) | ((val & 0xF0) >> 4));
        break;
    case 0x36:
        val = mmap.read_u16(get_register(Registers::HL));
        mmap.write_u8(get_register(Registers::HL), (val & 0xF) << 4);
        mmap.write_u8(get_register(Registers::HL), mmap.read_u16(get_register(Registers::HL)) | ((val & 0xF0) >> 4));
        break;
    case 0x37:
        val = get_register(Registers::A);
        set_register(Registers::A, (val & 0xF) << 4);
        set_register(Registers::A, get_register(Registers::A) | ((val & 0xF0) >> 4));
        break;

    default:
        val = 0;
        break;
    }

    set_register_bit(Registers::F, FlagRegisters::z, val == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
}

void Cpu::nop()
{
    std::cout << "nop" << std::endl;
}

// Decimal Adjust a
// Converts A into packed BCD.
void Cpu::daa()
{
    uint8_t a_val = get_register(Registers::A);
    if (get_register_bit(Registers::F, FlagRegisters::n))
    {
        if (get_register_bit(Registers::F, FlagRegisters::c))
            a_val -= 0x60;
        if (get_register_bit(Registers::F, FlagRegisters::h))
            a_val -= 0x6;
    }
    else
    {
        if (get_register_bit(Registers::F, FlagRegisters::c) || a_val > 0x99)
        {
            a_val += 0x60;
            set_register_bit(Registers::F, FlagRegisters::c, 1);
        }
        if (get_register_bit(Registers::F, FlagRegisters::h) || ((a_val & 0x0F) > 0x09))
            a_val += 0x6;
    }
    set_register_bit(Registers::F, FlagRegisters::z, a_val == 0);
    set_register_bit(Registers::F, FlagRegisters::h, 1);
    set_register(Registers::A, a_val);
}

void Cpu::cpl()
{
    set_register(Registers::A, ~get_register(Registers::A));
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::h, 1);
}

void Cpu::scf()
{
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    set_register_bit(Registers::F, FlagRegisters::c, 1);
}

void Cpu::ccf()
{
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    set_register_bit(Registers::F, FlagRegisters::c, !get_register_bit(Registers::F, FlagRegisters::c));
}

void Cpu::stop() { pc += 1; }

void Cpu::halt() { halted = true; }

void Cpu::di() { interrupts = false; }

void Cpu::ei() { interrupts = true; }