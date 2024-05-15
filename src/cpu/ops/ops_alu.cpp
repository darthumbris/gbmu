#include "Cpu.hpp"

void Cpu::and_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xA0 ... 0xA7:
        and_a_r8();
        break;
    case 0xE6:
        and_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::xor_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xA8 ... 0xAF:
        xor_a_r8();
        break;
    case 0xEE:
        xor_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::or_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xB0 ... 0xB7:
        or_a_r8();
        break;
    case 0xF6:
        or_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::cp_(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xB8 ... 0xBF:
        cp_a_r8();
        break;
    case 0xFE:
        cp_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::and_a_r8() {}
void Cpu::xor_a_r8() {}
void Cpu::or_a_r8() {}
void Cpu::cp_a_r8() {}

void Cpu::and_a_imm8() {}
void Cpu::xor_a_imm8() {}
void Cpu::or_a_imm8() {}
void Cpu::cp_a_imm8() {}