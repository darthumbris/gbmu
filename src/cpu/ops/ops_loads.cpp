#include "Cpu.hpp"

void Cpu::ld(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x01:
        ld_r16_imm16(opcode);
        break;
    case 0x02:
        ld_r16_a(opcode);
        break;
    case 0x06:
        ld_r8_imm8(opcode);
        break;
    case 0x08:
        ld_imm16_sp();
        break;
    case 0x0A:
        ld_a_r16(opcode);
        break;
    case 0x0E:
        ld_r8_imm8(opcode);
        break;
    case 0x11:
        ld_r16_imm16(opcode);
        break;
    case 0x12:
        ld_r16_a(opcode);
        break;
    case 0x16:
        ld_r8_imm8(opcode);
        break;
    case 0x1A:
        ld_a_r16(opcode);
        break;
    case 0x1E:
        ld_r8_imm8(opcode);
        break;
    case 0x21:
        ld_r16_imm16(opcode);
        break;
    case 0x22:
        ld_i_hl_a();
        break;
    case 0x26:
        ld_r8_imm8(opcode);
        break;
    case 0x2A:
        ld_i_a_hl();
        break;
    case 0x2E:
        ld_r8_imm8(opcode);
        break;
    case 0x31:
        ld_r16_imm16(opcode);
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
        ld_r8_imm8(opcode);
        break;
    case 0x40 ... 0x75:
        ld_r8_r8(opcode);
        break;
    case 0x77:
        ld_r16_a(opcode);
        break;
    case 0x78 ... 0x7F:
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
    uint8_t val;
    switch (opcode & 0xF)
    {
    case 0x8:
    case 0x0:
        val = get_register(Registers::B);
        break;
    case 0x9:
    case 0x1:
        val = get_register(Registers::C);
        break;
    case 0xA:
    case 0x2:
        val = get_register(Registers::D);
        break;
    case 0xB:
    case 0x3:
        val = get_register(Registers::E);
        break;
    case 0xC:
    case 0x4:
        val = get_register(Registers::H);
        break;
    case 0xD:
    case 0x5:
        val = get_register(Registers::L);
        break;
    case 0xE:
    case 0x6:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0xF:
    case 0x7:
        val = get_register(Registers::A);
        break;

    default:
        break;
    }

    switch (opcode)
    {
    case 0x40 ... 0x47:
        set_register(Registers::B, val);
        break;
    case 0x48 ... 0x4F:
        set_register(Registers::C, val);
        break;
    case 0x50 ... 0x57:
        set_register(Registers::D, val);
        break;
    case 0x58 ... 0x5F:
        set_register(Registers::E, val);
        break;
    case 0x60 ... 0x67:
        set_register(Registers::H, val);
        break;
    case 0x68 ... 0x6F:
        set_register(Registers::L, val);
        break;
    case 0x77:
    case 0x70 ... 0x75:
        mmap.write_u8(get_register(Registers::HL), val);
        break;
    case 0x78 ... 0x7F:
        set_register(Registers::A, val);
        break;

    default:
        break;
    }
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
    mmap.write_u16(address, get_register(Registers::A));
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
    set_register(Registers::A, mmap.read_u16(address));
}

void Cpu::ld_r8_imm8(uint16_t opcode)
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;

    switch (opcode)
    {
    case 0x06:
        set_register(Registers::B, val);
        break;
    case 0x0E:
        set_register(Registers::C, val);
        break;
    case 0x16:
        set_register(Registers::D, val);
        break;
    case 0x1E:
        set_register(Registers::E, val);
        break;
    case 0x26:
        set_register(Registers::H, val);
        break;
    case 0x2E:
        set_register(Registers::L, val);
        break;
    case 0x3E:
        set_register(Registers::A, val);
        break;

    default:
        break;
    }
}

void Cpu::ld_hl_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    mmap.write_u8(get_register(Registers::HL), val);
}

void Cpu::ld_r16_imm16(uint16_t opcode)
{
    uint16_t val = mmap.read_u16(pc);
    pc += 2;
    switch (opcode)
    {
    case 0x01:
        set_register(Registers::BC, val);
        break;
    case 0x11:
        set_register(Registers::DE, val);
        break;
    case 0x21:
        set_register(Registers::HL, val);
        break;
    case 0x31:
        set_register(Registers::SP, val);
        break;

    default:
        break;
    }
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
    mmap.write_u16(addr, get_register(Registers::SP));
}

void Cpu::ld_hl_sp_imm8()
{
    uint8_t e8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t val = get_register(Registers::SP);
    set_register(Registers::HL, static_cast<uint16_t>(val + e8));
    set_register_bit(Registers::F, FlagRegisters::z, 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(e8, val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(e8, val));
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
    set_register(Registers::SP, get_register(Registers::HL));
}

void Cpu::ld_i_hl_a()
{
    mmap.write_u8(get_register(Registers::HL), get_register(Registers::A));
    uint16_t hl = get_register(Registers::HL);
    set_register(Registers::HL, hl + 1);
}

void Cpu::ld_i_a_hl()
{
    set_register(Registers::A, mmap.read_u8(get_register(Registers::HL)));
    uint16_t hl = get_register(Registers::HL);
    set_register(Registers::HL, hl + 1);
}

void Cpu::ld_d_hl_a()
{
    mmap.write_u8(get_register(Registers::HL), get_register(Registers::A));
    uint16_t hl = get_register(Registers::HL);
    set_register(Registers::HL, hl - 1);
}

void Cpu::ld_d_a_hl()
{
    set_register(Registers::A, mmap.read_u8(get_register(Registers::HL)));
    uint16_t hl = get_register(Registers::HL);
    set_register(Registers::HL, hl - 1);
}

void Cpu::pop_r16stk(uint16_t opcode)
{
    uint16_t val = get_register(Registers::SP);
    switch (opcode)
    {
    case 0xC1:
        set_register(Registers::BC, val);
        break;
    case 0xD1:
        set_register(Registers::DE, val);
        break;
    case 0xE1:
        set_register(Registers::HL, val);
        break;
    case 0xF1:
        set_register(Registers::AF, val);
        break;

    default:
        break;
    }
    set_register(Registers::SP, val + 2);
}

void Cpu::push_r16stk(uint16_t opcode)
{
    // Push ss
    uint16_t val;
    switch (opcode)
    {
    case 0xC5:
        val = get_register(Registers::BC);
        break;
    case 0xD5:
        val = get_register(Registers::DE);
        break;
    case 0xE5:
        val = get_register(Registers::HL);
        break;
    case 0xF5:
        val = get_register(Registers::AF);
        break;

    default:
        val = 0;
        break;
    }

    uint16_t sp = get_register(Registers::SP);

    //(SP - 1) <- ssh (high byte)
    mmap.write_u8(sp - 1, (uint8_t)((val & 0xFF00) >> 8));
    //(SP - 2) <- ssl (low byte)
    mmap.write_u8(sp - 2, (uint8_t)((val & 0xFF)));

    // SP <- (SP - 2)
    set_register(Registers::SP, sp - 2);
}
