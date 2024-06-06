template<Condition condition>
void ret_cond()
{
    bool cond = false;
    switch (condition)
    {
    case Condition::NotZeroFlag:
        if (get_flag(FlagRegisters::z))
        {
            cond = true;
        }
        break;
    case Condition::ZeroFlag:
        if (!get_flag(FlagRegisters::z))
        {
            cond = true;
        }
        break;
    case Condition::NotCarryFlag:
        if (get_flag(FlagRegisters::c))
        {
            cond = true;
        }
        break;
    case Condition::CarryFlag:
        if (!get_flag(FlagRegisters::c))
        {
            cond = true;
        }
        break;
    default:
        break;
    }
    if (cond) {
        set_cycle(2);
        return;
    }
    set_cycle(1);
    ret();
}

void ret()
{
    pc = mmap.read_u16(sp);
    sp += 2;
    set_cycle(4);
}

void reti()
{
    ret();
    interrupts = true;
}