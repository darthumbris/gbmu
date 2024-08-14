#include "Cpu.hpp"

void Cpu::call_imm16() {
	pc += 2;
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = mmap.read_u16(pc - 2);
}

void Cpu::call_cond_nz() {
	call_cond_imm16(condition::NotZeroFlag);
}
void Cpu::call_cond_z() {
	call_cond_imm16(condition::ZeroFlag);
}
void Cpu::call_cond_nc() {
	call_cond_imm16(condition::NotCarryFlag);
}
void Cpu::call_cond_c() {
	call_cond_imm16(condition::CarryFlag);
}

void Cpu::call_cond_imm16(condition condition) {
	pc += 2;
	bool cond = false;
	switch (condition) {
	case condition::NotZeroFlag:
		if (get_flag(flag_registers::z)) {
			cond = true;
		}
		break;
	case condition::ZeroFlag:
		if (!get_flag(flag_registers::z)) {
			cond = true;
		}
		break;
	case condition::NotCarryFlag:
		if (get_flag(flag_registers::c)) {
			cond = true;
		}
		break;
	case condition::CarryFlag:
		if (!get_flag(flag_registers::c)) {
			cond = true;
		}
		break;
	default:
		break;
	}
	if (cond) {
		return;
	}
	branched = true;
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = mmap.read_u16(pc - 2);
}