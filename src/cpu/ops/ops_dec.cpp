#include "Cpu.hpp"

void Cpu::dec(uint8_t opcode, Operand op_r)
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
    set_16bitregister(op_r.reg, get_16bitregister(op_r.reg) - 1);
}

void Cpu::dec_r8(Operand op_r)
{
    uint8_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        mmap.write_u8(get_16bitregister(Registers::HL), val - 1);
        set_flag(FlagRegisters::z, val == 0x01);
        set_flag(FlagRegisters::h, ((val & 0b00011111) == 0b00010000));
    }
    else
    {
        val = get_register(op_r.reg);
        set_register(op_r.reg, val - 1);
        set_flag(FlagRegisters::z, (val - 1) == 0x00);
        set_flag(FlagRegisters::h, (val & 0xF) == 0);
    }
    set_flag(FlagRegisters::n, 1);
}