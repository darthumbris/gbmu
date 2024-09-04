#include "Cpu.hpp"

void Cpu::xor_a_imm8() {
	uint8_t n8 = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	set_register(registers::F, 0);
	set_register(registers::A, a_val ^ n8);
	set_flag(flag_registers::z, (a_val ^ n8) == 0);
}

void Cpu::xor_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint8_t a_val = get_register(registers::A);
	a_val ^= val;
	set_register(registers::A, a_val);
	set_register(registers::F, 0);
	set_flag(flag_registers::z, a_val == 0);
}

void Cpu::or_a_imm8() {
	uint8_t n8 = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	set_register(registers::F, 0);
	set_register(registers::A, a_val | n8);
	set_flag(flag_registers::z, (a_val | n8) == 0);
}

void Cpu::or_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint8_t a_val = get_register(registers::A);
	set_register(registers::A, a_val | val);
	set_register(registers::F, 0);
	set_flag(flag_registers::z, (a_val | val) == 0);
}

void Cpu::cp_a_imm8() {
	uint8_t cp = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	set_flag(flag_registers::z, (a_val == cp));
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::c, a_val < cp);
	set_flag(flag_registers::h, (a_val & 0xf) < (cp & 0xf));
}

void Cpu::cp_a_r8_hl() {
	uint8_t a_val = get_register(registers::A);
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	set_flag(flag_registers::z, (a_val == val));
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::c, a_val < val);
	set_flag(flag_registers::h, (a_val & 0xf) < (val & 0xf));
}

void Cpu::and_a_imm8() {
	uint8_t n8 = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	uint8_t a_and = a_val & n8;
	set_register(registers::A, a_and);
	set_flag(flag_registers::z, a_and == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 1);
	set_flag(flag_registers::c, 0);
}

void Cpu::and_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint8_t a_val = get_register(registers::A);
	set_register(registers::A, a_val & val);
	set_flag(flag_registers::z, (a_val & val) == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 1);
	set_flag(flag_registers::c, 0);
}