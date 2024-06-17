template<uint16_t value>
void rst_tg3()
{
    sp -= 2;
    mmap.write_u16(sp, pc);

    pc = value;
    set_cycle(4);
}