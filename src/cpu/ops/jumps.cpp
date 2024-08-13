#include "Cpu.hpp"

void Cpu::jr_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	if (val > 127) {
		pc -= (uint16_t)(255 - val + 1);
	} else {
		pc += (uint16_t)val;
	}
}

void Cpu::jp_imm16() {
	uint16_t val;
	pc += 2;
	val = mmap.read_u16(pc - 2);
	pc = val;
}

void Cpu::jp_hl() {
	uint16_t val;
	val = (get_16bitregister(Registers::HL));
	pc = val;
}

void Cpu::jp_cond_nz() {jp_cond_imm16(Condition::NotZeroFlag);}
void Cpu::jp_cond_z() {jp_cond_imm16(Condition::ZeroFlag);}
void Cpu::jp_cond_nc() {jp_cond_imm16(Condition::NotCarryFlag);}
void Cpu::jp_cond_c() {jp_cond_imm16(Condition::CarryFlag);}

void Cpu::jp_cond_imm16(Condition condition) {
	bool offset = false;
	uint16_t val;
	pc += 2;
	val = mmap.read_u16(pc - 2);
	switch (condition) {
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
	if (offset) {
		branched = true;
		pc = val;
	} else {
	}
}