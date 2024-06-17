template<Registers rec>
void swap_r8()
{
    uint8_t val;
    if (rec == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        mmap.write_u8(get_16bitregister(Registers::HL), (val & 0xF) << 4);
        mmap.write_u8(get_16bitregister(Registers::HL), mmap.read_u8(get_16bitregister(Registers::HL)) | ((val & 0xF0) >> 4));
        set_cycle(4);
    }
    else
    {
        val = get_register(rec);
        set_register(rec, (val & 0xF) << 4);
        set_register(rec, get_register(rec) | ((val & 0xF0) >> 4));
        set_cycle(2);
    }
    set_flag(FlagRegisters::z, val == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::c, 0);
    set_flag(FlagRegisters::h, 0);
}

void nop()
{
    set_cycle(1);
}

// Decimal Adjust a Converts A into packed BCD.
void daa()
{
    uint8_t a_val = get_register(Registers::A);
    if (get_flag(FlagRegisters::n))
    {
        if (get_flag(FlagRegisters::c))
            a_val -= 0x60;
        if (get_flag(FlagRegisters::h))
            a_val -= 0x6;
    }
    else
    {
        if (get_flag(FlagRegisters::c) || a_val > 0x99)
        {
            a_val += 0x60;
            set_flag(FlagRegisters::c, 1);
        }
        if (get_flag(FlagRegisters::h) || ((a_val & 0x0F) > 0x09))
            a_val += 0x6;
    }
    set_flag(FlagRegisters::z, a_val == 0);
    set_flag(FlagRegisters::h, 1);
    set_register(Registers::A, a_val);
    set_cycle(1);
}

void cpl()
{
    set_register(Registers::A, ~get_register(Registers::A));
    set_flag(FlagRegisters::n, 1);
    set_flag(FlagRegisters::h, 1);
    set_cycle(1);
}

void scf()
{
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    set_flag(FlagRegisters::c, 1);
    set_cycle(1);
}

void ccf()
{
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, 0);
    set_flag(FlagRegisters::c, !get_flag(FlagRegisters::c));
    set_cycle(1);
}

void stop() { pc += 1; set_cycle(1); }

void halt() { halted = true; set_cycle(1); }

void di() { process_interrupts = false; set_cycle(1);}

void ei() { process_interrupts = true; set_cycle(1); }