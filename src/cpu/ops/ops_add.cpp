#include "Cpu.hpp"

void Cpu::add(uint16_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0x09:
        add_hl_r16(op_s);
        break;
    case 0x19:
        add_hl_r16(op_s);
        break;
    case 0x29:
        add_hl_r16(op_s);
        break;
    case 0x39:
        add_hl_r16(op_s);
        break;
    case 0x80 ... 0x87:
        add_a_r8(op_s);
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

void Cpu::adc(uint16_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0x88 ... 0x8F:
        adc_a_r8(op_s);
        break;
    case 0xCE:
        adc_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::add_a_r8(Operand op_s)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    set_register(Registers::A, a_val + val);
    set_flag(FlagRegisters::z, (val + a_val) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
}

void Cpu::add_a_imm8()
{
    uint16_t val = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_flag(FlagRegisters::z, (val + a_val) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
}

void Cpu::add_hl_r16(Operand op_s)
{
    uint16_t val = get_register(op_s.reg);
    uint16_t hl_val = get_register(Registers::HL);
    set_register(Registers::HL, val + hl_val);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, hl_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, hl_val));
}

void Cpu::add_sp_imm8()
{
    uint8_t e8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t sp_val = get_register(Registers::SP);
    set_register(Registers::SP, static_cast<uint16_t>(e8 + sp_val));
    set_flag(FlagRegisters::z, 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(e8, sp_val));
    set_flag(FlagRegisters::c, carry_flag_set(e8, sp_val));
}

void Cpu::adc_a_r8(Operand op_s)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
    set_register(Registers::A, a_val + val + get_flag(FlagRegisters::c));
    set_flag(FlagRegisters::z, get_register(Registers::A) == 0);
}

void Cpu::adc_a_imm8()
{
    uint16_t val = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
    set_register(Registers::A, a_val + val + get_flag(FlagRegisters::c));
    set_flag(FlagRegisters::z, get_register(Registers::A) == 0);
}