#include "Cpu.hpp"

void Cpu::bit_b3_r8(uint16_t opcode, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    set_register_bit(Registers::F, FlagRegisters::h, 1);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    if ((val & (1 << bit_loc)) == 0)
    {
        set_register_bit(Registers::F, FlagRegisters::z, 1);
    }
    else
    {
        set_register_bit(Registers::F, FlagRegisters::z, 0);
    }
}

void Cpu::res_b3_r8(uint16_t opcode, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    val |= 0b11111111 ^ (0b1 << bit_loc);
    if (op_s.reg == Registers::HL)
    {
        mmap.write_u8(get_register(Registers::HL), val);
    }
    else
    {
        set_register(op_s.reg, val);
    }
}

void Cpu::set_b3_r8(uint16_t opcode, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    val |= (1 << bit_loc);
    if (op_s.reg == Registers::HL)
    {
        mmap.write_u8(get_register(Registers::HL), val);
    }
    else
    {
        set_register(op_s.reg, val);
    }
}