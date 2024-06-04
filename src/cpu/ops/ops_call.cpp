#include "Cpu.hpp"

void Cpu::call(uint8_t opcode)
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
    bool cond = false;
    switch (c)
    {
    case Condition::NotZeroFlag:
        if (get_flag(FlagRegisters::z))
        {
            cond = true;
        }
        break;
    case Condition::ZeroFlag:
        if (!get_flag(FlagRegisters::z))
        {
            cond = true;
        }
        break;
    case Condition::NotCarryFlag:
        if (get_flag(FlagRegisters::c))
        {
            cond = true;
        }
        break;
    case Condition::CarryFlag:
        if (!get_flag(FlagRegisters::c))
        {
            cond = true;
        }
        break;
    default:
        break;
    }
    if (cond) {
        set_cycle(3);
        return;
    }
    sp -= 2;
    mmap.write_u16(sp, pc);
    pc = mmap.read_u16(pc - 2);
    set_cycle(6);
}
void Cpu::call_imm16()
{
    pc += 2;
    sp -= 2;
    mmap.write_u16(sp, pc);
    pc = mmap.read_u16(pc - 2);
    set_cycle(6);
}