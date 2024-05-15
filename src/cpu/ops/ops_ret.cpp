#include "Cpu.hpp"

void Cpu::ret(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xC0:
        ret_cond();
        break;
    case 0xC8:
        ret_cond();
        break;
    case 0xC9:
        ret();
        break;
    case 0xD0:
        ret_cond();
        break;
    case 0xD8:
        ret_cond();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::ret_cond() {}
void Cpu::ret() {}
void Cpu::reti() {}