#include "Cpu.hpp"

void Cpu::ret() {
	pc = mmap.read_u16(sp);
	sp += 2;
	//set_cycle(4);
}

void Cpu::reti() {
	ret();
	interruptor.enable_processing();
}