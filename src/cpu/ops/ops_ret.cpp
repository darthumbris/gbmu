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

void Cpu::ret_cond(Condition c) {}
void Cpu::ret() {}
void Cpu::reti() {}