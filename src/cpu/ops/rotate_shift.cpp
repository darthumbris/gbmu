#include "Cpu.hpp"

uint8_t Cpu::get_rlc(uint8_t val, bool reset) {
	uint8_t ret = (val << 1) | (val >> 7);
	set_flag(FlagRegisters::c, (ret >> 0) & 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

uint8_t Cpu::get_rrc(uint8_t val, bool reset) {
	uint8_t ret = (val >> 1) | (val << 7);
	set_flag(FlagRegisters::c, (ret >> 7) & 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

uint8_t Cpu::get_rr(uint8_t val, bool reset) {
	uint8_t ret = (val >> 1) | (get_flag(FlagRegisters::c) << 7);
	set_flag(FlagRegisters::c, val & 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

uint8_t Cpu::get_rl(uint8_t val, bool reset) {
	uint8_t ret;
	ret = (val << 1) | (get_flag(FlagRegisters::c) << 0);
	set_flag(FlagRegisters::c, ((val & 0x80) >> 7) == 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

void Cpu::rlca() {
	set_register(Registers::A, get_rlc(get_register(Registers::A), true));
}

void Cpu::rla() {
	set_register(Registers::A, get_rl(get_register(Registers::A), true));
}

void Cpu::rrca() {
	set_register(Registers::A, get_rrc(get_register(Registers::A), true));
}

void Cpu::rra() {
	set_register(Registers::A, get_rr(get_register(Registers::A), true));
}