#include "Cpu.hpp"

void Cpu::sub(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x90 ... 0x97:
        sub_a_r8(opcode);
        break;
    case 0xD6:
        sub_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::sbc(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x98 ... 0x9F:
        sbc_a_r8(opcode);
        break;
    case 0xDE:
        sbc_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::sub_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0x80:
        val = get_register(Registers::B);
        set_register(Registers::A, a_val - val);
        break;
    case 0x81:
        val = get_register(Registers::C);
        set_register(Registers::A, a_val - val);
        break;
    case 0x82:
        val = get_register(Registers::D);
        set_register(Registers::A, a_val - val);
        break;
    case 0x83:
        val = get_register(Registers::E);
        set_register(Registers::A, a_val - val);
        break;
    case 0x84:
        val = get_register(Registers::H);
        set_register(Registers::A, a_val - val);
        break;
    case 0x85:
        val = get_register(Registers::L);
        set_register(Registers::A, a_val - val);
        break;
    case 0x86:
        val = mmap.read_u8(get_register(Registers::B));
        set_register(Registers::A, a_val - val);
        break;
    case 0x87:
        val = get_register(Registers::A);
        set_register(Registers::A, a_val - val);
        break;

    default:
        break;
    }

    set_register_bit(Registers::F, FlagRegisters::z, (a_val - val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::h, ((get_register(Registers::A) ^ val ^ a_val) & 0x10) != 0);
    set_register_bit(Registers::F, FlagRegisters::c, val > a_val);
}
void Cpu::sub_a_imm8()
{
    uint16_t val = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);

    set_register_bit(Registers::F, FlagRegisters::z, (a_val - val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::h, ((get_register(Registers::A) ^ val ^ a_val) & 0x10) != 0);
    set_register_bit(Registers::F, FlagRegisters::c, val > a_val);

    set_register(Registers::A, a_val - val);
}

void Cpu::sbc_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0x98:
        val = get_register(Registers::B);
        break;
    case 0x99:
        val = get_register(Registers::C);
        break;
    case 0x9A:
        val = get_register(Registers::D);
        break;
    case 0x9B:
        val = get_register(Registers::E);
        break;
    case 0x9C:
        val = get_register(Registers::H);
        break;
    case 0x9D:
        val = get_register(Registers::L);
        break;
    case 0x9E:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0x9F:
        val = get_register(Registers::A);
        break;

    default:
        val = 0;
        break;
    }
    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::c, val > a_val);
    set_register(Registers::A, a_val - val - get_register_bit(Registers::F, FlagRegisters::c));
    set_register_bit(Registers::F, FlagRegisters::z, get_register(Registers::A) == 0);
    set_register_bit(Registers::F, FlagRegisters::h, ((get_register(Registers::A) ^ (a_val + 1) ^ val) & 0x10) != 0);
}

void Cpu::sbc_a_imm8()
{
    uint16_t val = mmap.read_u8(pc) + get_register_bit(Registers::F, FlagRegisters::c);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);

    set_register_bit(Registers::F, FlagRegisters::n, 1);
    set_register_bit(Registers::F, FlagRegisters::z, (a_val - val) == 0);
    set_register_bit(Registers::F, FlagRegisters::c, val > a_val);
    set_register(Registers::A, a_val - val);
    set_register_bit(Registers::F, FlagRegisters::h, ((get_register(Registers::A) ^ (a_val + 1) ^ val) & 0x10) != 0);
}