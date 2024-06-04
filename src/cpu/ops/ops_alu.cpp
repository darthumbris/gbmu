#include "Cpu.hpp"

void Cpu::and_(uint8_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0xA0 ... 0xA7:
        and_a_r8(op_s);
        break;
    case 0xE6:
        and_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::xor_(uint8_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0xA8 ... 0xAF:
        xor_a_r8(op_s);
        break;
    case 0xEE:
        xor_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::or_(uint8_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0xB0 ... 0xB7:
        or_a_r8(op_s);
        break;
    case 0xF6:
        or_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::cp_(uint8_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0xB8 ... 0xBF:
        cp_a_r8(op_s);
        break;
    case 0xFE:
        cp_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::and_a_r8(Operand op_s)
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(2);
    }
    else
    {
        val = get_register(op_s.reg);
        set_cycle(1);
    }
    set_register(Registers::A, a_val & val);
    set_flag(FlagRegisters::z, (a_val & val) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 1);
    set_flag(FlagRegisters::c, 0);
}

void Cpu::and_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_register(Registers::A, a_val & n8);
    set_flag(FlagRegisters::z, (a_val & n8) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 1);
    set_flag(FlagRegisters::c, 0);
    set_cycle(2);
}

void Cpu::xor_a_r8(Operand op_s)
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(2);
    }
    else
    {
        val = get_register(op_s.reg);
        set_cycle(1);
    }
    a_val ^= val;
    set_register(Registers::A, a_val);
    set_register(Registers::F, 0);
    set_flag(FlagRegisters::z, a_val == 0);
}

void Cpu::or_a_r8(Operand op_s)
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(2);
    }
    else
    {
        val = get_register(op_s.reg);
        set_cycle(1);
    }
    set_register(Registers::A, a_val | val);
    set_register(Registers::F, 0);
    set_flag(FlagRegisters::z, (a_val | val) == 0);
}
void Cpu::cp_a_r8(Operand op_s)
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(2);
    }
    else
    {
        val = get_register(op_s.reg);
        set_cycle(1);
    }
    set_flag(FlagRegisters::z, (a_val == val));
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::c, a_val < val);
    set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
}

void Cpu::xor_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_register(Registers::F, 0);
    set_register(Registers::A, a_val ^ n8);
    set_flag(FlagRegisters::z, (a_val ^ n8) == 0);
    set_cycle(2);
}
void Cpu::or_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_register(Registers::F, 0);
    set_register(Registers::A, a_val | n8);
    set_flag(FlagRegisters::z, (a_val | n8) == 0);
    set_cycle(2);
}
void Cpu::cp_a_imm8()
{
    uint8_t cp = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_flag(FlagRegisters::z, (a_val == cp));
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::c, a_val < cp);
    set_flag(FlagRegisters::h, (a_val & 0xf) < (cp & 0xf));
    set_cycle(2);
}