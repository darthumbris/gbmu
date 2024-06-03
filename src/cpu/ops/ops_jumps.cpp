#include "Cpu.hpp"

void Cpu::jr(uint8_t opcode)
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

void Cpu::jp(uint8_t opcode)
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
    if (val > 127) {
        pc -= (uint16_t)(255 - val + 1);
    }
    else {
        pc += (uint16_t)val;
    }
}

void Cpu::jr_cond_imm8(Condition c)
{
    bool offset = false;
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    switch (c)
    {
    case Condition::NotZeroFlag:
        offset = get_flag(FlagRegisters::z) == 0;
        break;
    case Condition::ZeroFlag:
        offset = get_flag(FlagRegisters::z) == 1;
        break;
    case Condition::NotCarryFlag:
        offset = get_flag(FlagRegisters::c) == 0;
        break;
    case Condition::CarryFlag:
        offset = get_flag(FlagRegisters::c) == 1;
        break;
    default:
        break;
    }
    if ((debug_count > 24576 && debug_count < 24580) || (debug_count > 24627 && debug_count < 24631)) {
        // std::cout << "0x20 offset: " << offset << ", val: " << (uint16_t)val << std::endl;
        // std::bitset<8> x(get_register(Registers::F));
        // std::cout << "0x20 flag: 0b" << x << std::endl;
    }
    if (offset)
    {
        if (val > 127) {
            pc -= (uint16_t)(255 - val + 1);
        }
        else {
            pc += (uint16_t)val;
        }
    }
    else
    {
        // pc += (uint16_t)val;
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
        offset = get_flag(FlagRegisters::z) == 0;
        break;
    case Condition::ZeroFlag:
        offset = get_flag(FlagRegisters::z) == 1;
        break;
    case Condition::NotCarryFlag:
        offset = get_flag(FlagRegisters::c) == 0;
        break;
    case Condition::CarryFlag:
        offset = get_flag(FlagRegisters::c) == 1;
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
    val = mmap.read_u16(get_16bitregister(Registers::HL));
    pc = val;
}