#include "Cpu.hpp"

void Cpu::and_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xA0 ... 0xA7:
        and_a_r8(opcode);
        break;
    case 0xE6:
        and_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::xor_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xA8 ... 0xAF:
        xor_a_r8(opcode);
        break;
    case 0xEE:
        xor_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::or_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xB0 ... 0xB7:
        or_a_r8(opcode);
        break;
    case 0xF6:
        or_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::cp_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xB8 ... 0xBF:
        cp_a_r8(opcode);
        break;
    case 0xFE:
        cp_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::and_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0xA0:
        val = get_register(Registers::B);
        break;
    case 0xA1:
        val = get_register(Registers::C);
        break;
    case 0xA2:
        val = get_register(Registers::D);
        break;
    case 0xA3:
        val = get_register(Registers::E);
        break;
    case 0xA4:
        val = get_register(Registers::H);
        break;
    case 0xA5:
        val = get_register(Registers::L);
        break;
    case 0xA6:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0xA7:
        val = get_register(Registers::A);
        break;

    default:
        val = 0;
        break;
    }
    set_register(Registers::A, a_val & val);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val & val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 1);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
}

void Cpu::and_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_register(Registers::A, a_val & n8);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val & n8) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 1);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
}

void Cpu::xor_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0xA8:
        val = get_register(Registers::B);
        break;
    case 0xA9:
        val = get_register(Registers::C);
        break;
    case 0xAA:
        val = get_register(Registers::D);
        break;
    case 0xAB:
        val = get_register(Registers::E);
        break;
    case 0xAC:
        val = get_register(Registers::H);
        break;
    case 0xAD:
        val = get_register(Registers::L);
        break;
    case 0xAE:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0xAF:
        val = get_register(Registers::A);
        break;

    default:
        val = 0;
        break;
    }
    set_register(Registers::A, a_val ^ val);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val ^ val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
}
void Cpu::or_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0xB0:
        val = get_register(Registers::B);
        break;
    case 0xB1:
        val = get_register(Registers::C);
        break;
    case 0xB2:
        val = get_register(Registers::D);
        break;
    case 0xB3:
        val = get_register(Registers::E);
        break;
    case 0xB4:
        val = get_register(Registers::H);
        break;
    case 0xB5:
        val = get_register(Registers::L);
        break;
    case 0xB6:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0xB7:
        val = get_register(Registers::A);
        break;

    default:
        val = 0;
        break;
    }
    set_register(Registers::A, a_val | val);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val | val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
}
void Cpu::cp_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0xB8:
        val = get_register(Registers::B);
        break;
    case 0xB9:
        val = get_register(Registers::C);
        break;
    case 0xBA:
        val = get_register(Registers::D);
        break;
    case 0xBB:
        val = get_register(Registers::E);
        break;
    case 0xBC:
        val = get_register(Registers::H);
        break;
    case 0xBD:
        val = get_register(Registers::L);
        break;
    case 0xBE:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0xBF:
        val = get_register(Registers::A);
        break;

    default:
        val = 0;
        break;
    }
    set_register_bit(Registers::F, FlagRegisters::z, (a_val == val));
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::c, a_val < val);
    set_register_bit(Registers::F, FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
}

void Cpu::xor_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_register(Registers::A, a_val ^ n8);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val ^ n8) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
}
void Cpu::or_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_register(Registers::A, a_val | n8);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val | n8) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, 0);
    set_register_bit(Registers::F, FlagRegisters::c, 0);
}
void Cpu::cp_a_imm8()
{
    uint8_t cp = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val == cp));
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::c, a_val < cp);
    set_register_bit(Registers::F, FlagRegisters::h, (a_val & 0xf) < (cp & 0xf));
}