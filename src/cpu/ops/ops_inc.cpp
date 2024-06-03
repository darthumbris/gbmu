#include "Cpu.hpp"

void Cpu::inc(uint8_t opcode, Operand op_r)
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
    set_16bitregister(op_r.reg, get_16bitregister(op_r.reg) + 1);
}

void Cpu::inc_r8(Operand op_r)
{
    uint8_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        mmap.write_u8(get_16bitregister(Registers::HL), val + 1);
    }
    else
    {
        val = get_register(op_r.reg);
        set_register(op_r.reg, val + 1);
    }
    set_flag(FlagRegisters::z, (val + 1) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, (val & 0xF) == 0xF);
}