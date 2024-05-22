#include "Cpu.hpp"

void Cpu::inc(uint16_t opcode, Operand op_r)
{
    switch (opcode)
    {
    case 0x03:
        inc_r16(op_r);
        break;
    case 0x04:
        inc_r8(op_r);
        break;
    case 0x0C:
        inc_r8(op_r);
        break;
    case 0x13:
        inc_r16(op_r);
        break;
    case 0x14:
        inc_r8(op_r);
        break;
    case 0x1C:
        inc_r8(op_r);
        break;
    case 0x23:
        inc_r16(op_r);
        break;
    case 0x24:
        inc_r8(op_r);
        break;
    case 0x2C:
        inc_r8(op_r);
        break;
    case 0x33:
        inc_r16(op_r);
        break;
    case 0x34:
        inc_r8(op_r);
        break;
    case 0x3C:
        inc_r8(op_r);
        break;

    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::inc_r16(Operand op_r)
{
    set_register(op_r.reg, get_register(op_r.reg) + 1);
}

void Cpu::inc_r8(Operand op_r)
{
    uint16_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u16(get_register(Registers::HL));
        mmap.write_u16(get_register(Registers::HL), val + 1);
    }
    else
    {
        val = get_register(op_r.reg);
        set_register(op_r.reg, val + 1);
    }
    set_register_bit(Registers::F, FlagRegisters::z, (val + 1) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, 1));
}