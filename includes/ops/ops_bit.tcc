template<uint8_t opcode, Registers src>
void bit_b3_r8()
{
    uint8_t val;
    if (src == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(3);
    }
    else
    {
        val = get_register(src);
        set_cycle(2);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    set_flag(FlagRegisters::h, 1);
    set_flag(FlagRegisters::n, 0);
    if ((val & (1 << bit_loc)) == 0)
    {
        set_flag(FlagRegisters::z, 1);
    }
    else
    {
        set_flag(FlagRegisters::z, 0);
    }
}

template<uint8_t opcode, Registers src>
void res_b3_r8()
{
    uint8_t val;
    if (src == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(4);
    }
    else
    {
        val = get_register(src);
        set_cycle(2);
    }
    uint8_t bit_loc = ((opcode - 0x80) >> 3);
    // val |= 0b11111111 ^ (0b1 << bit_loc);
    //TODO check here
    val &= ~(1 << bit_loc);
    if (src == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(src, val);
    }
}

template<uint8_t opcode, Registers src>
void set_b3_r8()
{
    // std::cout << "Setting opcode: " << (uint16_t)opcode << std::endl;
    uint8_t val;
    if (src == Registers::HL)
    {
        val = mmap.read_u8(get_16bitregister(Registers::HL));
        set_cycle(4);
    }
    else
    {
        val = get_register(src);
        set_cycle(2);
    }
    uint8_t bit_loc = (opcode >> 3) & 0x7;
    val |= (1 << bit_loc);
    if (src == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
    }
    else
    {
        set_register(src, val);
    }
}