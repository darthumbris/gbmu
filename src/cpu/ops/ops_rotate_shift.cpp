#include "Cpu.hpp"

uint8_t Cpu::get_rlc(uint8_t val, bool reset)
{
    uint8_t ret = (val << 1) | (val >> 7);
    set_flag(FlagRegisters::c, (ret >> 0) & 1);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    if (reset)
    {
        set_flag(FlagRegisters::z, 0);
    }
    else {
        set_flag(FlagRegisters::z, ret == 0);
    }
    return ret;
}

uint8_t Cpu::get_rrc(uint8_t val, bool reset)
{
    uint8_t ret = (val >> 1) | (val << 7);
    set_flag(FlagRegisters::c, (ret >> 7) & 1);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    if (reset)
    {
        set_flag(FlagRegisters::z, 0);
    }
    else {
        set_flag(FlagRegisters::z, ret == 0);
    }
    return ret;
}

uint8_t Cpu::get_rr(uint8_t val, bool reset)
{
    uint8_t ret = (val >> 1) | (get_flag(FlagRegisters::c) << 7);
    set_flag(FlagRegisters::c, val & 1);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    if (reset)
    {
        set_flag(FlagRegisters::z, 0);
    }
    else {
        set_flag(FlagRegisters::z, ret == 0);
    }
    return ret;
}

uint8_t Cpu::get_rl(uint8_t val, bool reset)
{
    uint8_t ret;
    ret = (val << 1) | (get_flag(FlagRegisters::c) << 0);
    set_flag(FlagRegisters::c, ((val & 0x80) >> 7) == 1);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    if (reset)
    {
        set_flag(FlagRegisters::z, 0);
    }
    else {
        set_flag(FlagRegisters::z, ret == 0);
    }
    return ret;
}

void Cpu::rlca()
{
    set_register(Registers::A, get_rlc(get_register(Registers::A), true));
}

void Cpu::rla()
{
    set_register(Registers::A, get_rl(get_register(Registers::A), true));
}

void Cpu::rrca()
{
    set_register(Registers::A, get_rrc(get_register(Registers::A), true));
}

void Cpu::rra()
{
    set_register(Registers::A, get_rr(get_register(Registers::A), true));
}

void Cpu::rlc_r8(uint8_t opcode, Operand op_r)
{
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), get_rlc(mmap.read_u8(get_16bitregister(Registers::HL))));
    }
    else
    {
        set_register(op_r.reg, get_rlc(get_register(op_r.reg)));
    }
}

void Cpu::rl_r8(uint8_t opcode, Operand op_r)
{
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), get_rl(mmap.read_u8(get_16bitregister(Registers::HL))));
    }
    else
    {
        set_register(op_r.reg, get_rl(get_register(op_r.reg)));
    }
}

void Cpu::rrc_r8(uint8_t opcode, Operand op_r)
{
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), get_rrc(mmap.read_u8(get_16bitregister(Registers::HL))));
    }
    else
    {
        set_register(op_r.reg, get_rrc(get_register(op_r.reg)));
    }
}

void Cpu::rr_r8(uint8_t opcode, Operand op_r)
{
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), get_rr(mmap.read_u8(get_16bitregister(Registers::HL))));
    }
    else
    {
        set_register(op_r.reg, get_rr(get_register(op_r.reg)));
    }
}

void Cpu::sla_r8(uint8_t opcode, Operand op_r)
{
    uint8_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_r.reg);
    }
    set_flag(FlagRegisters::c, (val >> 7) & 0x01);
    val <<= 1;
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    set_flag(FlagRegisters::z, val == 0);
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(op_r.reg, val);
    }
}

void Cpu::sra_r8(uint8_t opcode, Operand op_r)
{
    uint8_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_r.reg);
    }
    set_flag(FlagRegisters::c, (val >> 0) & 1);
    val = (val >> 1) | (val & 0x80);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    set_flag(FlagRegisters::z, val == 0);
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(op_r.reg, val);
    }
}

void Cpu::srl_r8(uint8_t opcode, Operand op_r)
{
    uint8_t val;
    if (op_r.reg == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_r.reg);
    }
    set_flag(FlagRegisters::c, val & 0x01);
    val >>= 1;
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    set_flag(FlagRegisters::z, val == 0);
    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(op_r.reg, val);
    }
}
