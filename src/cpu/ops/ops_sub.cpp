#include "Cpu.hpp"

void Cpu::sub(uint16_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0x90 ... 0x97:
        sub_a_r8(op_s);
        break;
    case 0xD6:
        sub_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::sbc(uint16_t opcode, Operand op_s)
{
    switch (opcode)
    {
    case 0x98 ... 0x9F:
        sbc_a_r8(op_s);
        break;
    case 0xDE:
        sbc_a_imm8();
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::sub_a_r8(Operand op_s)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::B));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    set_register(Registers::A, a_val - val);
    set_flag(FlagRegisters::z, (a_val - val) == 0);
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ val ^ a_val) & 0x10) != 0);
    set_flag(FlagRegisters::c, val > a_val);
}
void Cpu::sub_a_imm8()
{
    uint16_t val = mmap.read_u8(pc);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);

    set_flag(FlagRegisters::z, (a_val - val) == 0);
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ val ^ a_val) & 0x10) != 0);
    set_flag(FlagRegisters::c, val > a_val);

    set_register(Registers::A, a_val - val);
}

void Cpu::sbc_a_r8(Operand op_s)
{
    uint16_t val;
    uint16_t a_val = get_register(Registers::A);
    if (op_s.reg == Registers::HL)
    {
        val = mmap.read_u8(get_register(Registers::B));
    }
    else
    {
        val = get_register(op_s.reg);
    }
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::c, val > a_val);
    set_register(Registers::A, a_val - val - get_flag(FlagRegisters::c));
    set_flag(FlagRegisters::z, get_register(Registers::A) == 0);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ (a_val + 1) ^ val) & 0x10) != 0);
}

void Cpu::sbc_a_imm8()
{
    uint16_t val = mmap.read_u8(pc) + get_flag(FlagRegisters::c);
    pc += 1;
    uint16_t a_val = get_register(Registers::A);

    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::z, (a_val - val) == 0);
    set_flag(FlagRegisters::c, val > a_val);
    set_register(Registers::A, a_val - val);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ (a_val + 1) ^ val) & 0x10) != 0);
}