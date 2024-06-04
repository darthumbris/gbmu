#include "Cpu.hpp"

void Cpu::ret(uint8_t opcode)
{
    switch (opcode)
    {
    case 0xC0:
        ret_cond(Condition::NotZeroFlag);
        break;
    case 0xC8:
        ret_cond(Condition::ZeroFlag);
        break;
    case 0xC9:
        ret();
        break;
    case 0xD0:
        ret_cond(Condition::NotCarryFlag);
        break;
    case 0xD8:
        ret_cond(Condition::CarryFlag);
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::ret_cond(Condition c)
{
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
        set_cycle(2);
        return;
    }
    set_cycle(1);
    ret();
}
void Cpu::ret()
{
    pc = mmap.read_u16(sp);
    sp += 2;
    set_cycle(4);
}
void Cpu::reti()
{
    ret();
    interrupts = true;
}