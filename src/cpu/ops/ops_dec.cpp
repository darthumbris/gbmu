#include "Cpu.hpp"

void Cpu::dec(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x05:
        dec_r8();
        break;
    case 0x0B:
        dec_r16();
        break;
    case 0x0D:
        dec_r8();
        break;
    case 0x15:
        dec_r8();
        break;
    case 0x1B:
        dec_r16();
        break;
    case 0x1D:
        dec_r8();
        break;
    case 0x25:
        dec_r8();
        break;
    case 0x2B:
        dec_r16();
        break;
    case 0x2D:
        dec_r8();
        break;
    case 0x35:
        dec_r8();
        break;
    case 0x3B:
        dec_r16();
        break;
    case 0x3D:
        dec_r8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::dec_r16() {}

void Cpu::dec_r8() {}