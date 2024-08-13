#include "Cpu.hpp"

void Cpu::xor_a_imm8() {
	uint8_t n8 = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	set_register(Registers::F, 0);
	set_register(Registers::A, a_val ^ n8);
	set_flag(FlagRegisters::z, (a_val ^ n8) == 0);
}

void Cpu::xor_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint8_t a_val = get_register(Registers::A);
	a_val ^= val;
	set_register(Registers::A, a_val);
	set_register(Registers::F, 0);
	set_flag(FlagRegisters::z, a_val == 0);
}

void Cpu::or_a_imm8() {
	uint8_t n8 = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	set_register(Registers::F, 0);
	set_register(Registers::A, a_val | n8);
	set_flag(FlagRegisters::z, (a_val | n8) == 0);
}

void Cpu::or_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint8_t a_val = get_register(Registers::A);
	set_register(Registers::A, a_val | val);
	set_register(Registers::F, 0);
	set_flag(FlagRegisters::z, (a_val | val) == 0);
}

void Cpu::cp_a_imm8() {
	uint8_t cp = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	set_flag(FlagRegisters::z, (a_val == cp));
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::c, a_val < cp);
	set_flag(FlagRegisters::h, (a_val & 0xf) < (cp & 0xf));
}

void Cpu::cp_a_r8_hl() {
	DEBUG_MSG("comparing val: %u from address %#06X to val: %u\n", mmap.read_u8(get_16bitregister(Registers::HL)),
	          get_16bitregister(Registers::HL), a_val);
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint8_t a_val = get_register(Registers::A);
	set_flag(FlagRegisters::z, (a_val == val));
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::c, a_val < val);
	set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
}

void Cpu::and_a_imm8() {
	uint8_t n8 = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	uint8_t a_and = a_val & n8;
	set_register(Registers::A, a_and);
	set_flag(FlagRegisters::z, a_and == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 1);
	set_flag(FlagRegisters::c, 0);
}

void Cpu::and_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	DEBUG_MSG("checking and with val: %u from addr: %#06X and val: %u\n", val, get_16bitregister(Registers::HL), a_val);
	uint8_t a_val = get_register(Registers::A);
	set_register(Registers::A, a_val & val);
	set_flag(FlagRegisters::z, (a_val & val) == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 1);
	set_flag(FlagRegisters::c, 0);
}