#include "Cpu.hpp"
#include "Operand.hpp"

void Cpu::ld_hl_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	mmap.write_u8(get_16bitregister(Registers::HL), val);
	set_cycle(3);
}

void Cpu::ld_a_imm16() {
	uint16_t addr = mmap.read_u16(pc);
	pc += 2;
	set_register(Registers::A, mmap.read_u8(addr));
	set_cycle(4);
}

void Cpu::ld_imm16_a() {
	uint16_t addr = mmap.read_u16(pc);
	pc += 2;
	mmap.write_u8(addr, get_register(Registers::A));
	set_cycle(4);
}

void Cpu::ld_imm16_sp() {
	uint16_t addr = mmap.read_u16(pc);
	pc += 2;
	mmap.write_u16(addr, sp);
	set_cycle(5);
}

void Cpu::ld_hl_sp_imm8() {
	int8_t e8 = mmap.read_u8(pc);
	pc += 1;
	uint16_t val = get_16bitregister(Registers::SP);
	set_16bitregister(Registers::HL, static_cast<uint16_t>(e8 + val));
	set_flag(FlagRegisters::z, 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, ((e8 & 0xF) + (val & 0xF)) > 0xf);
	set_flag(FlagRegisters::c, ((e8 & 0xFF) + (val & 0xFF)) > 0xff);
	set_cycle(3);
}

void Cpu::ldh_a_imm8() {
	uint8_t addr = mmap.read_u8(pc);
	pc += 1;
	set_register(Registers::A, mmap.read_u8(0xFF00 + static_cast<uint16_t>(addr)));
	set_cycle(3);
}

void Cpu::ldh_imm8_a() {
	uint8_t addr = mmap.read_u8(pc);
	pc += 1;
	mmap.write_u8(0xFF00 + static_cast<uint16_t>(addr), get_register(Registers::A));
	set_cycle(3);
}

void Cpu::ld_c_a() {
	mmap.write_u8(0xFF00 + static_cast<uint16_t>(get_register(Registers::C)), get_register(Registers::A));
	set_cycle(2);
}

void Cpu::ld_a_c() {
	set_register(Registers::A, mmap.read_u8(0xFF00 + static_cast<uint16_t>(get_register(Registers::C))));
	set_cycle(2);
}

void Cpu::ld_sp_hl() {
	sp = get_16bitregister(Registers::HL);
	set_cycle(2);
}

void Cpu::ld_i_hl_a() {
	mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl + 1);
	set_cycle(2);
}

void Cpu::ld_i_a_hl() {
	set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl + 1);
	set_cycle(2);
}

void Cpu::ld_d_hl_a() {
	mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl - 1);
	set_cycle(2);
}

void Cpu::ld_d_a_hl() {
	set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl - 1);
	set_cycle(2);
}