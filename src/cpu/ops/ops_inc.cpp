#include "Cpu.hpp"

void Cpu::inc(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x03:
        inc_r16();
        break;
    case 0x04:
        inc_r8();
        break;
    case 0x0C:
        inc_r8();
        break;
    case 0x13:
        inc_r16();
        break;
    case 0x14:
        inc_r8();
        break;
    case 0x1C:
        inc_r8();
        break;
    case 0x23:
        inc_r16();
        break;
    case 0x24:
        inc_r8();
        break;
    case 0x2C:
        inc_r8();
        break;
    case 0x33:
        inc_r16();
        break;
    case 0x34:
        inc_r8();
        break;
    case 0x3C:
        inc_r8();
        break;

    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::inc_r16() {}
void Cpu::inc_r8() {}