#include "Cpu.hpp"

void Cpu::ld(uint8_t opcode, std::vector<Operand> operands)
{
    switch (opcode)
    {
    case 0x01:
        ld_r16_imm16(operands[0]);
        break;
    case 0x02:
        ld_r16_a(operands[0]);
        break;
    case 0x06:
        ld_r8_imm8(operands[0]);
        break;
    case 0x08:
        ld_imm16_sp();
        break;
    case 0x0A:
        ld_a_r16(operands[1]);
        break;
    case 0x0E:
        ld_r8_imm8(operands[0]);
        break;
    case 0x11:
        ld_r16_imm16(operands[0]);
        break;
    case 0x12:
        ld_r16_a(operands[0]);
        break;
    case 0x16:
        ld_r8_imm8(operands[0]);
        break;
    case 0x1A:
        ld_a_r16(operands[1]);
        break;
    case 0x1E:
        ld_r8_imm8(operands[0]);
        break;
    case 0x21:
        ld_r16_imm16(operands[0]);
        break;
    case 0x22:
        ld_i_hl_a();
        break;
    case 0x26:
        ld_r8_imm8(operands[0]);
        break;
    case 0x2A:
        ld_i_a_hl();
        break;
    case 0x2E:
        ld_r8_imm8(operands[0]);
        break;
    case 0x31:
        ld_r16_imm16(operands[0]);
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
        ld_r8_imm8(operands[0]);
        break;
    case 0x40 ... 0x75:
        ld_r8_r8(operands[0], operands[1]);
        break;
    case 0x77:
        ld_r16_a(operands[0]);
        break;
    case 0x78 ... 0x7F:
        ld_r8_r8(operands[0], operands[1]);
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

void Cpu::ldh(uint8_t opcode)
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

void Cpu::ld_r8_r8(Operand op_r, Operand op_s)
{
    uint8_t val;
    if (op_s.reg == Registers::HL)
    {
        mmap.read_u8(get_16bitregister(Registers::HL));
    }
    else
    {
        val = get_register(op_s.reg);
    }

    if (op_r.reg == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(op_r.reg, val);
    }
}

void Cpu::ld_r16_a(Operand op_r)
{
    uint16_t address = get_16bitregister(op_r.reg);
    mmap.write_u8(address, get_register(Registers::A));
}

void Cpu::ld_a_r16(Operand op_s)
{
    uint16_t address = get_16bitregister(op_s.reg);
    set_register(Registers::A, mmap.read_u8(address));
}

void Cpu::ld_r8_imm8(Operand op_r)
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    set_register(op_r.reg, val);
}

void Cpu::ld_hl_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    mmap.write_u8(get_16bitregister(Registers::HL), val);
}

void Cpu::ld_r16_imm16(Operand op_r)
{
    uint16_t val = mmap.read_u16(pc);
    pc += 2;
    set_16bitregister(op_r.reg, val);
}

void Cpu::ld_a_imm16()
{
    uint16_t addr = mmap.read_u16(pc);
    pc += 2;
    set_register(Registers::A, mmap.read_u8(addr));
}

void Cpu::ld_imm16_a()
{
    uint16_t addr = mmap.read_u16(pc);
    pc += 2;
    mmap.write_u8(addr, get_register(Registers::A));
}

void Cpu::ld_imm16_sp()
{
    uint16_t addr = mmap.read_u16(pc);
    pc += 2;
    mmap.write_u16(addr, sp);
}

void Cpu::ld_hl_sp_imm8()
{
    uint8_t e8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t val = sp;
    set_16bitregister(Registers::HL, static_cast<uint16_t>(val + e8));
    set_flag(FlagRegisters::z, 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(e8, val));
    set_flag(FlagRegisters::c, carry_flag_set(e8, val));
}

void Cpu::ldh_a_imm8()
{
    uint16_t addr = mmap.read_u8(pc);
    pc += 1;
    set_register(Registers::A, mmap.read_u8(0xFF00 + addr));
}

void Cpu::ldh_imm8_a()
{
    uint16_t addr = mmap.read_u8(pc);
    pc += 1;
    mmap.write_u8(0xFF00 + addr, get_register(Registers::A));
}

void Cpu::ld_c_a()
{
    mmap.write_u8(0xFF00 + get_register(Registers::C), get_register(Registers::A));
}

void Cpu::ld_a_c()
{
    set_register(Registers::A, mmap.read_u8(0xFF00 + get_register(Registers::C)));
}

void Cpu::ld_sp_hl()
{
    sp = get_16bitregister(Registers::HL);
}

void Cpu::ld_i_hl_a()
{
    mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl + 1);
}

void Cpu::ld_i_a_hl()
{
    set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl + 1);
}

void Cpu::ld_d_hl_a()
{
    mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl - 1);
}

void Cpu::ld_d_a_hl()
{
    set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl - 1);
}

void Cpu::pop_r16stk(Operand op_r)
{
    uint16_t val = sp;
    set_16bitregister(op_r.reg, val);
    sp = val + 2;
}

void Cpu::push_r16stk(Operand op_s)
{
    // Push ss
    uint16_t val = get_16bitregister(op_s.reg);
    uint16_t sp_val = this->sp;

    //(SP - 1) <- ssh (high byte)
    mmap.write_u8(sp_val - 1, (uint8_t)((val & 0xFF00) >> 8));
    //(SP - 2) <- ssl (low byte)
    mmap.write_u8(sp_val - 2, (uint8_t)((val & 0xFF)));

    // SP <- (SP - 2)
    sp = sp_val - 2;
}
