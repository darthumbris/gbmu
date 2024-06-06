template<Registers src>
void sub_a_r8()
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (src == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(2);
    }
    else
    {
        val = get_register(src);
        set_cycle(1);
    }
    set_register(Registers::A, a_val - val);
    set_flag(FlagRegisters::z, (a_val - val) == 0);
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ val ^ a_val) & 0x10) != 0);
    set_flag(FlagRegisters::c, val > a_val);
}

void sub_a_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);

    set_flag(FlagRegisters::z, (a_val - val) == 0);
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ val ^ a_val) & 0x10) != 0);
    set_flag(FlagRegisters::c, val > a_val);

    set_register(Registers::A, a_val - val);
    set_cycle(2);
}

template<Registers src>
void sbc_a_r8()
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (src == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(2);
    }
    else
    {
        val = get_register(src);
        set_cycle(1);
    }
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::c, val > a_val);
    set_register(Registers::A, a_val - val - get_flag(FlagRegisters::c));
    set_flag(FlagRegisters::z, get_register(Registers::A) == 0);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ (a_val + 1) ^ val) & 0x10) != 0);
}

void sbc_a_imm8()
{
    uint8_t val = mmap.read_u8(pc) + get_flag(FlagRegisters::c);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);

    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::z, (a_val - val) == 0);
    set_flag(FlagRegisters::c, val > a_val);
    set_register(Registers::A, a_val - val);
    set_flag(FlagRegisters::h, ((get_register(Registers::A) ^ (a_val + 1) ^ val) & 0x10) != 0);
    set_cycle(2);
}