#include "Cpu.hpp"

void Cpu::jr_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	if (val > 127) {
		pc -= (uint16_t)(255 - val + 1);
	} else {
		pc += (uint16_t)val;
	}
	set_cycle(3);
}

void Cpu::jp_imm16() {
	uint16_t val;
	pc += 2;
	val = mmap.read_u16(pc - 2);
	pc = val;
	set_cycle(4);
}

void Cpu::jp_hl() {
	uint16_t val;
	val = (get_16bitregister(Registers::HL));
	pc = val;
	set_cycle(1);
}