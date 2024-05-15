#include "Cpu.hpp"

void Cpu::call(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xC4:
        call_cond_imm16();
        break;
    case 0xCC:
        call_cond_imm16();
        break;
    case 0xCD:
        call_imm16();
        break;
    case 0xD4:
        call_cond_imm16();
        break;
    case 0xDC:
        call_cond_imm16();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::call_cond_imm16() {}
void Cpu::call_imm16() {}