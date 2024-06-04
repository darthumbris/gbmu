#include "Cpu.hpp"
#include <string>
#include <iostream>

void Cpu::rst_tg3(Instruction in)
{
    sp -= 2;
    mmap.write_u16(sp, pc);

    pc = in.operands[0].value.value();
    set_cycle(4);
    // std::cout << "PC set to: 0x" << std::setfill('0') << std::setw(4) << std::hex << pc << " operand.name: " << in.operands[0].name << std::dec << std::endl;
}