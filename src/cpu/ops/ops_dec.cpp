#include "Cpu.hpp"

void Cpu::dec(uint16_t opcode, Operand op_r)
{
    switch (opcode)
    {
    case 0x05:
        dec_r8(op_r);
        break;
    case 0x0B:
        dec_r16(op_r);
        break;
    case 0x0D:
        dec_r8(op_r);
        break;
    case 0x15:
        dec_r8(op_r);
        break;
    case 0x1B:
        dec_r16(op_r);
        break;
    case 0x1D:
        dec_r8(op_r);
        break;
    case 0x25:
        dec_r8(op_r);
        break;
    case 0x2B:
        dec_r16(op_r);
        break;
    case 0x2D:
        dec_r8(op_r);
        break;
    case 0x35:
        dec_r8(op_r);
        break;
    case 0x3B:
        dec_r16(op_r);
        break;
    case 0x3D:
        dec_r8(op_r);
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::dec_r16(Operand op_r)
{
    set_register(op_r.reg, get_register(op_r.reg) - 1);
}

void Cpu::dec_r8(Operand op_r)
{
    uint16_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u16(get_register(Registers::HL));
        mmap.write_u16(get_register(Registers::HL), val - 1);
    }
    else
    {
        val = get_register(op_r.reg);
        set_register(op_r.reg, val - 1);
    }
    set_register_bit(Registers::F, FlagRegisters::z, (val - 1) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, -1)); // TODO check this correct
}