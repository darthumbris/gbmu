template<Registers rec>
void inc_r16()
{
    set_16bitregister(rec, get_16bitregister(rec) + 1);
    set_cycle(2);
}

template<Registers rec>
void inc_r8()
{
    uint8_t val;
    if (rec == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        mmap.write_u8(get_16bitregister(Registers::HL), val + 1);
        set_cycle(3);
    }
    else
    {
        val = get_register(rec);
        set_register(rec, val + 1);
        set_cycle(1);
    }
    set_flag(FlagRegisters::z, get_register(rec) == 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, (get_register(rec) & 0xF) == 0x00);
}