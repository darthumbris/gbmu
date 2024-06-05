template<uint16_t value>
void rst_tg3()
{
    sp -= 2;
    mmap.write_u16(sp, pc);

    pc = value;
    set_cycle(4);
    // std::cout << "PC set to: 0x" << std::setfill('0') << std::setw(4) << std::hex << pc << " operand.name: " << in.operands[0].name << std::dec << std::endl;
}