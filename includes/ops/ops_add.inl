template<Registers src>
void add_a_r8()
{
    uint8_t val;
    uint8_t a_val = get_register(Registers::A);
    if (src == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(1);
    }
    else
    {
        val = get_register(src);
        set_cycle(1);
    }
    set_register(Registers::A, a_val + val);
    set_flag(FlagRegisters::z, (val + a_val) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
}

void add_a_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_flag(FlagRegisters::z, (val + a_val) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
    set_cycle(1);
}

template<Registers src>
void add_hl_r16()
{
    uint16_t val = get_16bitregister(src);
    uint16_t hl_val = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, val + hl_val);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, hl_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, hl_val));
    set_cycle(2);
}

void add_sp_imm8()
{
    uint8_t e8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t sp_val = sp;
    sp = static_cast<uint16_t>(e8 + sp_val);
    set_flag(FlagRegisters::z, 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(e8, sp_val));
    set_flag(FlagRegisters::c, carry_flag_set(e8, sp_val));
    set_cycle(4);
}

template<Registers src>
void adc_a_r8()
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
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::z, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
    set_register(Registers::A, a_val + val + get_flag(FlagRegisters::c));
    // set_flag(FlagRegisters::z, get_register(Registers::A) == 0);
}

void adc_a_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    uint8_t a_val = get_register(Registers::A);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::z, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(val, a_val));
    set_flag(FlagRegisters::c, carry_flag_set(val, a_val));
    set_register(Registers::A, a_val + val + get_flag(FlagRegisters::c));
    set_cycle(2);
    // set_flag(FlagRegisters::z, get_register(Registers::A) == 0);
}