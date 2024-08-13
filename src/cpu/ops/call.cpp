#include "Cpu.hpp"

void Cpu::call_imm16() {
	pc += 2;
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = mmap.read_u16(pc - 2);
}

void Cpu::call_cond_nz() {call_cond_imm16(Condition::NotZeroFlag);}
void Cpu::call_cond_z() {call_cond_imm16(Condition::ZeroFlag);}
void Cpu::call_cond_nc() {call_cond_imm16(Condition::NotCarryFlag);}
void Cpu::call_cond_c() {call_cond_imm16(Condition::CarryFlag);}

void Cpu::call_cond_imm16(Condition condition) {
	pc += 2;
	bool cond = false;
	switch (condition) {
	case Condition::NotZeroFlag:
		if (get_flag(FlagRegisters::z)) {
			cond = true;
		}
		break;
	case Condition::ZeroFlag:
		if (!get_flag(FlagRegisters::z)) {
			cond = true;
		}
		break;
	case Condition::NotCarryFlag:
		if (get_flag(FlagRegisters::c)) {
			cond = true;
		}
		break;
	case Condition::CarryFlag:
		if (!get_flag(FlagRegisters::c)) {
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