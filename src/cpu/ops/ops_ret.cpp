#include "Cpu.hpp"

void Cpu::ret(uint16_t opcode)
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
    ret();
}
void Cpu::ret()
{
    pc = mmap.read_u16(get_register(Registers::SP));
    set_register(Registers::SP, get_register(Registers::SP) + 2);
}
void Cpu::reti()
{
    ret();
    interrupts = true;
}