#include "Cpu.hpp"

void Cpu::jr(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x18:
        jr_imm8();
        break;
    case 0x20:
        jr_cond_imm8();
        break;
    case 0x28:
        jr_cond_imm8();
        break;
    case 0x30:
        jr_cond_imm8();
        break;
    case 0x38:
        jr_cond_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::jp(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xC2:
        jp_cond_imm16();
        break;
    case 0xC3:
        jp_imm16();
        break;
    case 0xCA:
        jp_cond_imm16();
        break;
    case 0xD2:
        jp_cond_imm16();
        break;
    case 0xDA:
        jp_cond_imm16();
        break;
    case 0xE9:
        jp_hl();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::jr_imm8() {}
void Cpu::jr_cond_imm8() {}

void Cpu::jp_cond_imm16() {}
void Cpu::jp_imm16() {}
void Cpu::jp_hl() {}