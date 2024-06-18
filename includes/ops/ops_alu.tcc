template<Registers src>
void and_a_r8()
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
    set_register(Registers::A, a_val & val);
    set_flag(FlagRegisters::z, (a_val & val) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 1);
    set_flag(FlagRegisters::c, 0);
}

void and_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    uint8_t a_and = a_val & n8;
    set_register(Registers::A, a_and);
    set_flag(FlagRegisters::z, a_and == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 1);
    set_flag(FlagRegisters::c, 0);
    set_cycle(2);
}

template<Registers src>
void xor_a_r8()
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
    a_val ^= val;
    set_register(Registers::A, a_val);
    set_register(Registers::F, 0);
    set_flag(FlagRegisters::z, a_val == 0);
}

template<Registers src>
void or_a_r8()
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
    set_register(Registers::A, a_val | val);
    set_register(Registers::F, 0);
    set_flag(FlagRegisters::z, (a_val | val) == 0);
}

template<Registers src>
void cp_a_r8()
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
    set_flag(FlagRegisters::z, (a_val == val));
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::c, a_val < val);
    set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
}

void xor_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_register(Registers::F, 0);
    set_register(Registers::A, a_val ^ n8);
    set_flag(FlagRegisters::z, (a_val ^ n8) == 0);
    set_cycle(2);
}

void or_a_imm8()
{
    uint8_t n8 = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_register(Registers::F, 0);
    set_register(Registers::A, a_val | n8);
    set_flag(FlagRegisters::z, (a_val | n8) == 0);
    set_cycle(2);
}

void cp_a_imm8()
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