#include "Cpu.hpp"

void Cpu::call(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xC4:
        call_cond_imm16(Condition::NotZeroFlag);
        break;
    case 0xCC:
        call_cond_imm16(Condition::ZeroFlag);
        break;
    case 0xCD:
        call_imm16();
        break;
    case 0xD4:
        call_cond_imm16(Condition::NotCarryFlag);
        break;
    case 0xDC:
        call_cond_imm16(Condition::CarryFlag);
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::call_cond_imm16(Condition c)
{
    pc += 2;
    switch (c)
    {
    case Condition::NotZeroFlag:
        if (get_register_bit(Registers::F, FlagRegisters::z))
        {
            return;
        }
        break;
    case Condition::ZeroFlag:
        if (!get_register_bit(Registers::F, FlagRegisters::z))
        {
            return;
        }
        break;
    case Condition::NotCarryFlag:
        if (get_register_bit(Registers::F, FlagRegisters::c))
        {
            return;
        }
        break;
    case Condition::CarryFlag:
        if (!get_register_bit(Registers::F, FlagRegisters::c))
        {
            return;
        }
        break;
    default:
        break;
    }
    set_register(Registers::SP, get_register(Registers::SP) - 2);
    mmap.write_u16(get_register(Registers::SP), pc);
    pc = mmap.read_u16(pc - 2);
}
void Cpu::call_imm16()
{
    pc += 2;
    set_register(Registers::SP, get_register(Registers::SP) - 2);
    mmap.write_u16(get_register(Registers::SP), pc);
    pc = mmap.read_u16(pc - 2);
}