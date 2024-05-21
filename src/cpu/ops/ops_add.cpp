#include "Cpu.hpp"

void Cpu::add(Instruction in, uint16_t opcode)
{
    switch (opcode)
    {
    case 0x09:
        add_hl_r16(opcode);
        break;
    case 0x19:
        add_hl_r16(opcode);
        break;
    case 0x29:
        add_hl_r16(opcode);
        break;
    case 0x39:
        add_hl_r16(opcode);
        break;
    case 0x80 ... 0x87:
        add_a_r8(opcode);
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
        adc_a_r8(opcode);
        break;
    case 0xCE:
        adc_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::add_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0x80:
        val = get_register(Registers::B);
        set_register(Registers::A, a_val + val);
        break;
    case 0x81:
        val = get_register(Registers::C);
        set_register(Registers::A, a_val + val);
        break;
    case 0x82:
        val = get_register(Registers::D);
        set_register(Registers::A, a_val + val);
        break;
    case 0x83:
        val = get_register(Registers::E);
        set_register(Registers::A, a_val + val);
        break;
    case 0x84:
        val = get_register(Registers::H);
        set_register(Registers::A, a_val + val);
        break;
    case 0x85:
        val = get_register(Registers::L);
        set_register(Registers::A, a_val + val);
        break;
    case 0x86:
        val = mmap.read_u8(get_register(Registers::B));
        set_register(Registers::A, a_val + val);
        break;
    case 0x87:
        val = get_register(Registers::A);
        set_register(Registers::A, a_val + val);
        break;

    default:
        break;
    }

    set_register_bit(Registers::F, FlagRegisters::z, (val + a_val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(val, a_val));
}

void Cpu::add_a_imm8()
{
    uint16_t val = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_register_bit(Registers::F, FlagRegisters::z, (val + a_val) == 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(val, a_val));
}

void Cpu::add_hl_r16(uint16_t opcode)
{
    uint16_t val;
    uint16_t hl_val = get_register(Registers::HL);
    switch (opcode)
    {
    case 0x09:
        val = get_register(Registers::BC);
        set_register(Registers::HL, val + hl_val);
        break;
    case 0x19:
        val = get_register(Registers::DE);
        set_register(Registers::HL, val + hl_val);
        break;
    case 0x29:
        val = get_register(Registers::HL);
        set_register(Registers::HL, val + hl_val);
        break;

    default:
        val = 0;
        break;
    }
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, hl_val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(val, hl_val));
}

void Cpu::add_sp_imm8()
{
    uint8_t e8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t sp_val = get_register(Registers::SP);
    set_register(Registers::SP, static_cast<uint16_t>(e8 + sp_val));
    set_register_bit(Registers::F, FlagRegisters::z, 0);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(e8, sp_val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(e8, sp_val));
}

void Cpu::adc_a_r8(uint16_t opcode)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    switch (opcode)
    {
    case 0x88:
        val = get_register(Registers::B);
        break;
    case 0x89:
        val = get_register(Registers::C);
        break;
    case 0x8A:
        val = get_register(Registers::D);
        break;
    case 0x8B:
        val = get_register(Registers::E);
        break;
    case 0x8C:
        val = get_register(Registers::H);
        break;
    case 0x8D:
        val = get_register(Registers::L);
        break;
    case 0x8E:
        val = mmap.read_u8(get_register(Registers::HL));
        break;
    case 0x8F:
        val = get_register(Registers::A);
        break;

    default:
        val = 0;
        break;
    }
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(val, a_val));
    set_register(Registers::A, a_val + val + get_register_bit(Registers::F, FlagRegisters::c));
    set_register_bit(Registers::F, FlagRegisters::z, get_register(Registers::A) == 0);
}

void Cpu::adc_a_imm8()
{
    uint16_t val = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);
    set_register_bit(Registers::F, FlagRegisters::n, 0);
    set_register_bit(Registers::F, FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_register_bit(Registers::F, FlagRegisters::c, carry_flag_set(val, a_val));
    set_register(Registers::A, a_val + val + get_register_bit(Registers::F, FlagRegisters::c));
    set_register_bit(Registers::F, FlagRegisters::z, get_register(Registers::A) == 0);
}