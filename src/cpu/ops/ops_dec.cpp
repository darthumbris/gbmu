#include "Cpu.hpp"

void Cpu::dec(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x05:
        dec_r8(opcode);
        break;
    case 0x0B:
        dec_r16(opcode);
        break;
    case 0x0D:
        dec_r8(opcode);
        break;
    case 0x15:
        dec_r8(opcode);
        break;
    case 0x1B:
        dec_r16(opcode);
        break;
    case 0x1D:
        dec_r8(opcode);
        break;
    case 0x25:
        dec_r8(opcode);
        break;
    case 0x2B:
        dec_r16(opcode);
        break;
    case 0x2D:
        dec_r8(opcode);
        break;
    case 0x35:
        dec_r8(opcode);
        break;
    case 0x3B:
        dec_r16(opcode);
        break;
    case 0x3D:
        dec_r8(opcode);
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::dec_r16(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x0B:
        set_register(Registers::BC, get_register(Registers::BC) - 1);
        break;
    case 0x1B:
        set_register(Registers::DE, get_register(Registers::DE) - 1);
        break;
    case 0x2B:
        set_register(Registers::HL, get_register(Registers::HL) - 1);
        break;
    case 0x3B:
        set_register(Registers::SP, get_register(Registers::SP) - 1);
        break;
    default:
        break;
    }
}

void Cpu::dec_r8(uint16_t opcode)
{
    uint16_t val;
    switch (opcode)
    {
    case 0x05:
        val = get_register(Registers::B);
        set_register(Registers::B, val - 1);
        break;
    case 0x0D:
        val = get_register(Registers::C);
        set_register(Registers::C, val - 1);
        break;
    case 0x15:
        val = get_register(Registers::D);
        set_register(Registers::D, val - 1);
        break;
    case 0x1D:
        val = get_register(Registers::E);
        set_register(Registers::E, val - 1);
        break;
    case 0x25:
        val = get_register(Registers::H);
        set_register(Registers::H, val - 1);
        break;
    case 0x2D:
        val = get_register(Registers::L);
        set_register(Registers::L, val - 1);
        break;
    case 0x35:
        val = mmap.read_u16(get_register(Registers::HL));
        mmap.write_u16(get_register(Registers::HL), val - 1);
        break;
    case 0x3D:
        val = get_register(Registers::SP);
        set_register(Registers::SP, val - 1);
        break;
    default:
        break;
    }
    set_register_bit(Registers::F, FlagRegisters::z, (val - 1) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, -1)); // TODO check this correct
}