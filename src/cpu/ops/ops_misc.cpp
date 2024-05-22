#include "Cpu.hpp"
#include <iostream>

void Cpu::swap_r8(uint16_t opcode, Operand op_r)
{
    uint16_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u16(get_register(Registers::HL));
        mmap.write_u8(get_register(Registers::HL), (val & 0xF) << 4);
        mmap.write_u8(get_register(Registers::HL), mmap.read_u16(get_register(Registers::HL)) | ((val & 0xF0) >> 4));
    }
    else
    {
        val = get_register(op_r.reg);
        set_register(op_r.reg, (val & 0xF) << 4);
        set_register(op_r.reg, get_register(op_r.reg) | ((val & 0xF0) >> 4));
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