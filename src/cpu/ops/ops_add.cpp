#include "Cpu.hpp"

void Cpu::add(Instruction in, uint16_t opcode)
{
    switch (opcode)
    {
    case 0x09:
        add_hl_r16();
        break;
    case 0x19:
        add_hl_r16();
        break;
    case 0x29:
        add_hl_r16();
        break;
    case 0x39:
        add_hl_r16();
        break;
    case 0x80 ... 0x87:
        add_a_r8();
        break;
    case 0xC6:
        add_a_imm8();
        break;
    case 0xE8:
        add_sp_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::adc(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x88 ... 0x8F:
        adc_a_r8();
        break;
    case 0xCE:
        adc_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::add_hl_r16() {}
void Cpu::add_a_r8() {}
void Cpu::add_a_imm8() {}
void Cpu::add_sp_imm8() {}

void Cpu::adc_a_r8() {}
void Cpu::adc_a_imm8() {}