#include "Cpu.hpp"

void Cpu::bit_b3_r8(uint8_t opcode, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    set_flag(FlagRegisters::h, 1);
    set_flag(FlagRegisters::n, 0);
    if ((val & (1 << bit_loc)) == 0)
    {
        set_flag(FlagRegisters::z, 1);
    }
    else
    {
        set_flag(FlagRegisters::z, 0);
    }
}

void Cpu::res_b3_r8(uint8_t opcode, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    val |= 0b11111111 ^ (0b1 << bit_loc);
    if (op_s.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(op_s.reg, val);
    }
}

void Cpu::set_b3_r8(uint8_t opcode, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    val |= (1 << bit_loc);
    if (op_s.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(op_s.reg, val);
    }
}