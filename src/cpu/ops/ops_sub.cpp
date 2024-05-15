#include "Cpu.hpp"

void Cpu::sub(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x90 ... 0x97:
        sub_a_r8();
        break;
    case 0xD6:
        sub_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::sbc(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x98 ... 0x9F:
        sbc_a_r8();
        break;
    case 0xDE:
        sbc_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::sub_a_r8() {}
void Cpu::sub_a_imm8() {}

void Cpu::sbc_a_r8() {}
void Cpu::sbc_a_imm8() {}