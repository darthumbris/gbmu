#include "Cpu.hpp"

void Cpu::inc(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x03:
        inc_r16(opcode);
        break;
    case 0x04:
        inc_r8(opcode);
        break;
    case 0x0C:
        inc_r8(opcode);
        break;
    case 0x13:
        inc_r16(opcode);
        break;
    case 0x14:
        inc_r8(opcode);
        break;
    case 0x1C:
        inc_r8(opcode);
        break;
    case 0x23:
        inc_r16(opcode);
        break;
    case 0x24:
        inc_r8(opcode);
        break;
    case 0x2C:
        inc_r8(opcode);
        break;
    case 0x33:
        inc_r16(opcode);
        break;
    case 0x34:
        inc_r8(opcode);
        break;
    case 0x3C:
        inc_r8(opcode);
        break;

    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::inc_r16(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x03:
        set_register(Registers::BC, get_register(Registers::BC) + 1);
        break;
    case 0x13:
        set_register(Registers::DE, get_register(Registers::DE) + 1);
        break;
    case 0x23:
        set_register(Registers::HL, get_register(Registers::HL) + 1);
        break;
    case 0x33:
        set_register(Registers::SP, get_register(Registers::SP) + 1);
        break;
    default:
        break;
    }
}

void Cpu::inc_r8(uint16_t opcode)
{
    uint16_t val;
    switch (opcode)
    {
    case 0x04:
        val = get_register(Registers::B);
        set_register(Registers::B, val + 1);
        break;
    case 0x0C:
        val = get_register(Registers::C);
        set_register(Registers::C, val + 1);
        break;
    case 0x14:
        val = get_register(Registers::D);
        set_register(Registers::D, val + 1);
        break;
    case 0x1C:
        val = get_register(Registers::E);
        set_register(Registers::E, val + 1);
        break;
    case 0x24:
        val = get_register(Registers::H);
        set_register(Registers::H, val + 1);
        break;
    case 0x2C:
        val = get_register(Registers::L);
        set_register(Registers::L, val + 1);
        break;
    case 0x34:
        val = mmap.read_u16(get_register(Registers::HL));
        mmap.write_u16(get_register(Registers::HL), val + 1);
        break;
    case 0x3C:
        val = get_register(Registers::SP);
        set_register(Registers::SP, val + 1);
        break;
    default:
        break;
    }
    set_register_bit(Registers::F, FlagRegisters::z, (val + 1) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, 1));
}