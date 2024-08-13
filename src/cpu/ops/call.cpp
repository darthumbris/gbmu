#include "Cpu.hpp"

void Cpu::call_imm16() {
	pc += 2;
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = mmap.read_u16(pc - 2);
}