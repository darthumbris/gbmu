void jr_imm8()
{
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    if (val > 127) {
        pc -= (uint16_t)(255 - val + 1);
    }
    else {
        pc += (uint16_t)val;
    }
    set_cycle(3);
}

template<Condition condition>
void jr_cond_imm8()
{
    bool offset = false;
    uint8_t val = mmap.read_u8(pc);
    pc += 1;
    switch (condition)
    {
    case Condition::NotZeroFlag:
        offset = get_flag(FlagRegisters::z) == 0;
        break;
    case Condition::ZeroFlag:
        offset = get_flag(FlagRegisters::z) == 1;
        break;
    case Condition::NotCarryFlag:
        offset = get_flag(FlagRegisters::c) == 0;
        break;
    case Condition::CarryFlag:
        offset = get_flag(FlagRegisters::c) == 1;
        break;
    default:
        break;
    }
    // if ((debug_count > 2218245 - 2 && debug_count < 2218245 + 2)) {
        // std::cout << "0x20 offset: " << offset << ", val: " << (uint16_t)val << " flag: " << (uint16_t)get_register(Registers::F) << std::endl;
        // std::bitset<8> x(get_register(Registers::F));
        // std::cout << "0x20 flag: 0b" << x << std::endl;
    // }
    if (offset)
    {
        if (val > 127) {
            pc -= (uint16_t)(255 - val + 1);
        }
        else {
            pc += (uint16_t)val;
        }
        set_cycle(3);
    }
    else
    {
        set_cycle(2);
    }
}

template<Condition condition>
void jp_cond_imm16()
{
    bool offset = false;
    uint16_t val;
    pc += 2;
    val = mmap.read_u16(pc - 2);
    switch (condition)
    {
    case Condition::NotZeroFlag:
        offset = get_flag(FlagRegisters::z) == 0;
        break;
    case Condition::ZeroFlag:
        offset = get_flag(FlagRegisters::z) == 1;
        break;
    case Condition::NotCarryFlag:
        offset = get_flag(FlagRegisters::c) == 0;
        break;
    case Condition::CarryFlag:
        offset = get_flag(FlagRegisters::c) == 1;
        break;
    default:
        break;
    }
    if (offset)
    {
        pc = val;
        set_cycle(4);
    }
    else {
        set_cycle(3);
    }
}

void jp_imm16()
{
    uint16_t val;
    pc += 2;
    val = mmap.read_u16(pc - 2);
    pc = val;
    set_cycle(4);
}

void jp_hl()
{
    uint16_t val;
    val = mmap.read_u16(get_16bitregister(Registers::HL));
    pc = val;
    set_cycle(1);
}