#include "Cpu.hpp"
#include <cstdint>

void Cpu::ret() {
	pc = mmap.read_u16(sp);
	sp += 2;
}

void Cpu::reti() {
	ret();
	interruptor.enable_processing();
}

void Cpu::rst_00() {
	rst_tg3(0x00);
}

void Cpu::rst_08() {
	rst_tg3(0x08);
}

void Cpu::rst_10() {
	rst_tg3(0x10);
}

void Cpu::rst_18() {
	rst_tg3(0x18);
}

void Cpu::rst_20() {
	rst_tg3(0x20);
}

void Cpu::rst_28() {
	rst_tg3(0x28);
}

void Cpu::rst_30() {
	rst_tg3(0x30);
}

void Cpu::rst_38() {
	rst_tg3(0x38);
}

void Cpu::ret_nz() {
	ret_cond(condition::NotZeroFlag);
}

void Cpu::ret_z() {
	ret_cond(condition::ZeroFlag);
}

void Cpu::ret_nc() {
	ret_cond(condition::NotCarryFlag);
}

void Cpu::ret_c() {
	ret_cond(condition::CarryFlag);
}

void Cpu::ret_cond(condition condition) {
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
	ret();
}

void Cpu::rst_tg3(uint16_t value) {
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = value;
}