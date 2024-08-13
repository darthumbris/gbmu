#include "Cpu.hpp"

void Cpu::ret() {
	pc = mmap.read_u16(sp);
	sp += 2;
}

void Cpu::reti() {
	ret();
	interruptor.enable_processing();
}