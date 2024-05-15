#include "Cpu.hpp"

void Cpu::ld(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x01:
        ld_r16_imm16();
        break;
    case 0x02:
        ld_r16_a(opcode);
        break;
    case 0x06:
        ld_r8_imm8();
        break;
    case 0x08:
        ld_imm16_sp();
        break;
    case 0x0A:
        ld_a_r16(opcode);
        break;
    case 0x0E:
        ld_r8_imm8();
        break;
    case 0x11:
        ld_r16_imm16();
        break;
    case 0x12:
        ld_r16_a(opcode);
        break;
    case 0x16:
        ld_r8_imm8();
        break;
    case 0x1A:
        ld_a_r16(opcode);
        break;
    case 0x1E:
        ld_r8_imm8();
        break;
    case 0x21:
        ld_r16_imm16();
        break;
    case 0x22:
        ld_i_hl_a();
        break;
    case 0x26:
        ld_r8_imm8();
        break;
    case 0x2A:
        ld_i_a_hl();
        break;
    case 0x2E:
        ld_r8_imm8();
        break;
    case 0x31:
        ld_r16_imm16();
        break;
    case 0x32:
        ld_d_hl_a();
        break;
    case 0x36:
        ld_hl_imm8();
        break;
    case 0x3A:
        ld_d_a_hl();
        break;
    case 0x3E:
        ld_r8_imm8();
        break;
    case 0x40 ... 0x75:
        ld_r8_r8(opcode);
        break;
    case 0x77:
        ld_r16_a(opcode);
        break;
    case 0x78 ... 0x7D:
        ld_r8_r8(opcode);
        break;
    case 0x7E:
        ld_a_r16(opcode);
        break;
    case 0x7F:
        ld_r8_r8(opcode);
        break;
    case 0xE2:
        ld_c_a();
        break;
    case 0xEA:
        ld_imm16_a();
        break;
    case 0xF2:
        ld_a_c();
        break;
    case 0xF8:
        ld_hl_sp_imm8();
        break;
    case 0xF9:
        ld_sp_hl();
        break;
    case 0xFA:
        ld_a_imm16();
        break;

    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::ldh(uint16_t opcode)
{
    switch (opcode)
    {
    case 0xE0:
        ldh_imm8_a();
        break;
    case 0xF0:
        ldh_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::ld_r8_r8(uint16_t opcode)
{
}

void Cpu::ld_r16_a(uint16_t opcode)
{
    uint16_t address;
    switch (opcode)
    {
    case 0x02:
        address = get_register(Registers::BC);
        break;
    case 0x12:
        address = get_register(Registers::DE);
        break;
    case 0x77:
        address = get_register(Registers::HL);
        break;

    default:
        address = 0;
        break;
    }
    // TODO write to address here the value of register A (need to make the mmu)
}

void Cpu::ld_a_r16(uint16_t opcode)
{
    uint16_t address;
    switch (opcode)
    {
    case 0x0A:
        address = get_register(Registers::BC);
        break;
    case 0x1A:
        address = get_register(Registers::DE);
        break;
    case 0x7E:
        address = get_register(Registers::HL);
        break;

    default:
        address = 0;
        break;
    }
    // uint16_t val = //TODO read the value from the mmu here
    uint16_t val;
    set_register(Registers::A, val);
}

void Cpu::ld_i_hl_a() {}
void Cpu::ld_d_hl_a() {}
void Cpu::ld_i_a_hl() {}
void Cpu::ld_d_a_hl() {}
void Cpu::ld_hl_imm8() {}
void Cpu::ld_c_a() {}
void Cpu::ld_a_c() {}
void Cpu::ld_r16_imm16() {}
void Cpu::ld_imm16_sp() {}
void Cpu::ld_r8_imm8() {}
void Cpu::ld_imm16_a() {}
void Cpu::ld_a_imm16() {}
void Cpu::ld_hl_sp_imm8() {}
void Cpu::ld_sp_hl() {}

void Cpu::ldh_c_a() {}
void Cpu::ldh_imm8_a() {}
void Cpu::ldh_a_c() {}
void Cpu::ldh_a_imm8() {}

void Cpu::pop_r16stk() {}
void Cpu::push_r16stk() {}