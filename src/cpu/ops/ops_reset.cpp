#include "Cpu.hpp"
#include <string>
#include <iostream>

void Cpu::rst_tg3(Instruction in)
{
    // registers[4] -= 2;
    set_register(Registers::SP, get_register(Registers::SP) - 2);
    mmap.write_u16(get_register(Registers::SP), pc);
    // set_register(Registers::SP, pc);

    pc = in.operands[0].value.value();
    // pc = std::stoi(in.operands[0].name.substr(1, 2), 0, 16);
    // std::cout << "PC set to: 0x" << std::setfill('0') << std::setw(4) << std::hex << pc << " operand.name: " << in.operands[0].name << std::dec << std::endl;
}