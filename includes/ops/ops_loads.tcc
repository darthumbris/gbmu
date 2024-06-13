template<Registers rec,Registers src>
void ld_r8_r8()
{
    uint8_t val;
    if (src == Registers::HL)
    {
        mmap.read_u8(get_16bitregister(Registers::HL));
        // set_cycle(2);
    }
    else
    {
        val = get_register(src);
        // set_cycle(1);
    }

    if (rec == Registers::HL)
    {
        mmap.write_u8(get_16bitregister(Registers::HL), val);
        set_cycle(2);
    }
    else
    {
        set_register(rec, val);
        set_cycle(1);
    }
}

template<Registers rec>
void ld_r16_a()
{
    uint16_t address = get_16bitregister(rec);
    mmap.write_u8(address, get_register(Registers::A));
    set_cycle(2);
}

template<Registers src>
void ld_a_r16()
{
    uint16_t address = get_16bitregister(src);
    set_register(Registers::A, mmap.read_u8(address));
    set_cycle(2);
}

template<Registers rec>
void ld_r8_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    set_register(rec, val);
    set_cycle(2);
}

void ld_hl_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    mmap.write_u8(get_16bitregister(Registers::HL), val);
    set_cycle(3);
}

template<Registers rec>
void ld_r16_imm16()
{
    uint16_t val = mmap.read_u16(pc);
    pc += 2;
    set_16bitregister(rec, val);
    set_cycle(3);
}

void ld_a_imm16()
{
    uint16_t addr = mmap.read_u16(pc);
    pc += 2;
    set_register(Registers::A, mmap.read_u8(addr));
    set_cycle(4);
}

void ld_imm16_a()
{
    uint16_t addr = mmap.read_u16(pc);
    pc += 2;
    mmap.write_u8(addr, get_register(Registers::A));
    set_cycle(4);
}

void ld_imm16_sp()
{
    uint16_t addr = mmap.read_u16(pc);
    pc += 2;
    mmap.write_u16(addr, sp);
    set_cycle(5);
}

void ld_hl_sp_imm8()
{
    uint8_t e8 = mmap.read_u8(pc);
    pc += 1;
    uint16_t val = sp;
    set_16bitregister(Registers::HL, static_cast<uint16_t>(val + e8));
    set_flag(FlagRegisters::z, 0);
    set_flag(FlagRegisters::n, 0);
    set_flag(FlagRegisters::h, half_carry_flag_set(e8, val));
    set_flag(FlagRegisters::c, carry_flag_set(e8, val));
    set_cycle(3);
}

void ldh_a_imm8()
{
    uint16_t addr = mmap.read_u8(pc);
    pc += 1;
    // if (debug_count == 2395650)
    // printf("addr: %#06x val: %u\n", 0xFF00 + addr, mmap.read_u8(0xFF00 + addr));
    // std::cout << "reading from addr: " << std::hex << (0xFF00 | addr) << std::dec << " val: " << (uint16_t)mmap.read_u8(0xFF00 | addr) << std::endl;
    set_register(Registers::A, mmap.read_u8(0xFF00 + addr));
    set_cycle(3);
}

void ldh_imm8_a()
{
    uint16_t addr = mmap.read_u8(pc);
    pc += 1;
    mmap.write_u8(0xFF00 + addr, get_register(Registers::A));
    set_cycle(3);
}

void ld_c_a()
{
    mmap.write_u8(0xFF00 + get_register(Registers::C), get_register(Registers::A));
    set_cycle(2);
}

void ld_a_c()
{
    // if (debug_count == 1650581) {
    //     std::cout << "reading from addr: 0x" << std::hex << (uint16_t)(0xFF00 + get_register(Registers::C)) << std::dec << " val: " << (uint16_t)mmap.read_u8(0xFF00 + get_register(Registers::C)) << std::endl;
    // }
    set_register(Registers::A, mmap.read_u8(0xFF00 + get_register(Registers::C)));
    set_cycle(2);
}

void ld_sp_hl()
{
    sp = get_16bitregister(Registers::HL);
    set_cycle(2);
}

void ld_i_hl_a()
{
    mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl + 1);
    set_cycle(2);
}

void ld_i_a_hl()
{
    set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl + 1);
    set_cycle(2);
}

void ld_d_hl_a()
{
    mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl - 1);
    set_cycle(2);
}

void ld_d_a_hl()
{
    set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
    uint16_t hl = get_16bitregister(Registers::HL);
    set_16bitregister(Registers::HL, hl - 1);
    set_cycle(2);
}

template<Registers rec>
INLINE_FN void pop_r16stk()
{
    uint16_t val = sp;
    set_16bitregister(rec, mmap.read_u16(sp));
    sp += 2;
    set_cycle(3);
}

template<Registers src>
INLINE_FN void push_r16stk()
{
    // Push ss
    uint16_t val = get_16bitregister(src);
    uint16_t sp_val = this->sp;

    //(SP - 1) <- ssh (high byte)
    mmap.write_u8(sp_val - 1, (uint8_t)((val & 0xFF00) >> 8));
    //(SP - 2) <- ssl (low byte)
    mmap.write_u8(sp_val - 2, (uint8_t)((val & 0xFF)));

    // SP <- (SP - 2)
    sp = sp_val - 2;
    set_cycle(4);
}
