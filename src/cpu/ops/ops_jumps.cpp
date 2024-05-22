#include "Cpu.hpp"

void Cpu::jr(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x18:
        jr_imm8();
        break;
    case 0x20:
        jr_cond_imm8(Condition::NotZeroFlag);
        break;
    case 0x28:
        jr_cond_imm8(Condition::ZeroFlag);
        break;
    case 0x30:
        jr_cond_imm8(Condition::NotCarryFlag);
        break;
    case 0x38:
        jr_cond_imm8(Condition::CarryFlag);
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::jp(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xC2:
        jp_cond_imm16(Condition::NotZeroFlag);
        break;
    case 0xC3:
        jp_imm16();
        break;
    case 0xCA:
        jp_cond_imm16(Condition::ZeroFlag);
        break;
    case 0xD2:
        jp_cond_imm16(Condition::NotCarryFlag);
        break;
    case 0xDA:
        jp_cond_imm16(Condition::CarryFlag);
        break;
    case 0xE9:
        jp_hl();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::jr_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    pc -= (uint16_t)(255 - val + 1);
}

void Cpu::jr_cond_imm8(Condition c)
{
    bool offset = false;
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    switch (c)
    {
    case Condition::NotZeroFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::z) == 0;
        break;
    case Condition::ZeroFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::z) == 1;
        break;
    case Condition::NotCarryFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::c) == 0;
        break;
    case Condition::CarryFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::c) == 1;
        break;
    default:
        break;
    }
    if (offset)
    {
        pc -= (uint16_t)(255 - val + 1);
    }
    else
    {
        pc += (uint16_t)val;
    }
}

void Cpu::jp_cond_imm16(Condition c)
{
    bool offset = false;
    uint16_t val;
    pc += 2;
    val = mmap.read_u16(pc - 2);
    switch (c)
    {
    case Condition::NotZeroFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::z) == 0;
        break;
    case Condition::ZeroFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::z) == 1;
        break;
    case Condition::NotCarryFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::c) == 0;
        break;
    case Condition::CarryFlag:
        offset = get_register_bit(Registers::F, FlagRegisters::c) == 1;
        break;
    default:
        break;
    }
    if (offset)
    {
        pc = val;
    }
}
void Cpu::jp_imm16()
{
    uint16_t val;
    pc += 2;
    val = mmap.read_u16(pc - 2);
    pc = val;
}
void Cpu::jp_hl()
{
    uint16_t val;
    val = mmap.read_u16(get_register(Registers::HL));
    pc = val;
}