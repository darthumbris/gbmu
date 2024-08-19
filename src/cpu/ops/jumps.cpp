#include "Cpu.hpp"

void Cpu::jr_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	if (val > 127) {
		pc -= static_cast<uint16_t>(255 - val + 1);
	} else {
		pc += static_cast<uint16_t>(val);
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
	val = (get_16bitregister(registers::HL));
	pc = val;
}

void Cpu::jp_cond_nz() {
	jp_cond_imm16(condition::NotZeroFlag);
}
void Cpu::jp_cond_z() {
	jp_cond_imm16(condition::ZeroFlag);
}
void Cpu::jp_cond_nc() {
	jp_cond_imm16(condition::NotCarryFlag);
}
void Cpu::jp_cond_c() {
	jp_cond_imm16(condition::CarryFlag);
}

void Cpu::jp_cond_imm16(condition cond) {
	bool offset = false;
	uint16_t val;
	pc += 2;
	val = mmap.read_u16(pc - 2);
	switch (cond) {
	case condition::NotZeroFlag:
		offset = get_flag(flag_registers::z) == 0;
		break;
	case condition::ZeroFlag:
		offset = get_flag(flag_registers::z) == 1;
		break;
	case condition::NotCarryFlag:
		offset = get_flag(flag_registers::c) == 0;
		break;
	case condition::CarryFlag:
		offset = get_flag(flag_registers::c) == 1;
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