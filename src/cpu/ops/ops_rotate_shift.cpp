#include "Cpu.hpp"

uint8_t Cpu::get_rlc(uint8_t val, bool reset)
{
    val = (val << 1) | (val >> 7);
    set_register_bit(Registers::F, FlagRegisters::c, (val >> 0) & 1);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    if (reset)
    {
        set_register_bit(Registers::F, FlagRegisters::z, 0);
    }
    return val;
}

uint8_t Cpu::get_rrc(uint8_t val, bool reset)
{
    val = (val >> 1) | (val << 7);
    set_register_bit(Registers::F, FlagRegisters::c, (val >> 7) & 1);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    if (reset)
    {
        set_register_bit(Registers::F, FlagRegisters::z, 0);
    }
    return val;
}

uint8_t Cpu::get_rr(uint8_t val, bool reset)
{
    set_register_bit(Registers::F, FlagRegisters::c, val & 1);
    val = (val >> 1) | (get_register_bit(Registers::F, FlagRegisters::c) << 7);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    if (reset)
    {
        set_register_bit(Registers::F, FlagRegisters::z, 0);
    }
    return val;
}

uint8_t Cpu::get_rl(uint8_t val, bool reset)
{
    set_register_bit(Registers::F, FlagRegisters::c, (val >> 7) & 1);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    if (reset)
    {
        set_register_bit(Registers::F, FlagRegisters::z, 0);
    }
    return (val << 1) | (get_register_bit(Registers::F, FlagRegisters::c) << 0);
}

void Cpu::rlca()
{
    set_register(Registers::A, get_rlc(get_register(Registers::A), true));
}

void Cpu::rla()
{
    set_register(Registers::A, get_rl(get_register(Registers::A), true));
}

void Cpu::rrca()
{
    set_register(Registers::A, get_rrc(get_register(Registers::A), true));
}

void Cpu::rra()
{
    set_register(Registers::A, get_rr(get_register(Registers::A), true));
}

void Cpu::rlc_r8(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x00:
        set_register(Registers::B, get_rlc(get_register(Registers::B)));
        break;
    case 0x01:
        set_register(Registers::C, get_rlc(get_register(Registers::C)));
        break;
    case 0x02:
        set_register(Registers::D, get_rlc(get_register(Registers::D)));
        break;
    case 0x03:
        set_register(Registers::E, get_rlc(get_register(Registers::E)));
        break;
    case 0x04:
        set_register(Registers::H, get_rlc(get_register(Registers::H)));
        break;
    case 0x05:
        set_register(Registers::L, get_rlc(get_register(Registers::L)));
        break;
    case 0x06:
        mmap.write_u8(get_register(Registers::HL), get_rlc(mmap.read_u8(get_register(Registers::HL))));
        break;
    case 0x07:
        set_register(Registers::A, get_rlc(get_register(Registers::A)));
        break;

    default:
        break;
    }
}

void Cpu::rl_r8(uint16_t opcode) {}

void Cpu::rrc_r8(uint16_t opcode) {}

void Cpu::rr_r8(uint16_t opcode) {}

void Cpu::sla_r8(uint16_t opcode) {}

void Cpu::sra_r8(uint16_t opcode) {}

void Cpu::srl_r8(uint16_t opcode) {}
