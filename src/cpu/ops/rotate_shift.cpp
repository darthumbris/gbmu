#include "Cpu.hpp"

uint8_t Cpu::get_rlc(uint8_t val, bool reset) {
	uint8_t ret = (val << 1) | (val >> 7);
	set_flag(flag_registers::c, (ret >> 0) & 1);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	if (reset) {
		set_flag(flag_registers::z, 0);
	} else {
		set_flag(flag_registers::z, ret == 0);
	}
	return ret;
}

uint8_t Cpu::get_rrc(uint8_t val, bool reset) {
	uint8_t ret = (val >> 1) | (val << 7);
	set_flag(flag_registers::c, (ret >> 7) & 1);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	if (reset) {
		set_flag(flag_registers::z, 0);
	} else {
		set_flag(flag_registers::z, ret == 0);
	}
	return ret;
}

uint8_t Cpu::get_rr(uint8_t val, bool reset) {
	uint8_t ret = (val >> 1) | (get_flag(flag_registers::c) << 7);
	set_flag(flag_registers::c, val & 1);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	if (reset) {
		set_flag(flag_registers::z, 0);
	} else {
		set_flag(flag_registers::z, ret == 0);
	}
	return ret;
}

uint8_t Cpu::get_rl(uint8_t val, bool reset) {
	uint8_t ret;
	ret = (val << 1) | (get_flag(flag_registers::c) << 0);
	set_flag(flag_registers::c, ((val & 0x80) >> 7) == 1);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	if (reset) {
		set_flag(flag_registers::z, 0);
	} else {
		set_flag(flag_registers::z, ret == 0);
	}
	return ret;
}

void Cpu::rlca() {
	set_register(registers::A, get_rlc(get_register(registers::A), true));
}

void Cpu::rla() {
	set_register(registers::A, get_rl(get_register(registers::A), true));
}

void Cpu::rrca() {
	set_register(registers::A, get_rrc(get_register(registers::A), true));
}

void Cpu::rra() {
	set_register(registers::A, get_rr(get_register(registers::A), true));
}

void Cpu::rlc_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	mmap.write_u8(get_16bitregister(registers::HL), get_rlc(read_cache));
}

void Cpu::rl_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	mmap.write_u8(get_16bitregister(registers::HL), get_rl(read_cache));
}

void Cpu::rrc_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	mmap.write_u8(get_16bitregister(registers::HL), get_rrc(read_cache));
}

void Cpu::rr_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	mmap.write_u8(get_16bitregister(registers::HL), get_rr(read_cache));
}

void Cpu::sla_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	set_flag(flag_registers::c, (read_cache >> 7) & 0x01);
	read_cache <<= 1;
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	set_flag(flag_registers::z, read_cache == 0);
	mmap.write_u8(get_16bitregister(registers::HL), read_cache);
}

void Cpu::sra_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	set_flag(flag_registers::c, (read_cache >> 0) & 1);
	read_cache = (read_cache >> 1) | (read_cache & 0x80);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	set_flag(flag_registers::z, read_cache == 0);
	mmap.write_u8(get_16bitregister(registers::HL), read_cache);
}

void Cpu::srl_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	set_flag(flag_registers::c, read_cache & 0x01);
	read_cache >>= 1;
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	set_flag(flag_registers::z, read_cache == 0);
	mmap.write_u8(get_16bitregister(registers::HL), read_cache);
}

void Cpu::swap_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL));
		return;
	}
	mmap.write_u8(get_16bitregister(registers::HL), (read_cache & 0xF) << 4);
	mmap.write_u8(get_16bitregister(registers::HL),
	              mmap.read_u8(get_16bitregister(registers::HL)) | ((read_cache & 0xF0) >> 4));
	set_flag(flag_registers::z, read_cache == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::c, 0);
	set_flag(flag_registers::h, 0);
}