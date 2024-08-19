#include "Cpu.hpp"

void Cpu::sub_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	uint16_t sub = a_val - val;
	set_flag(flag_registers::z, sub == 0);
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::h, (a_val & 0xf) < (val & 0xf));
	set_flag(flag_registers::c, val > a_val);
	set_register(registers::A, static_cast<uint8_t>(sub));
}

void Cpu::sub_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint8_t a_val = get_register(registers::A);
	uint16_t sub = a_val - val;
	set_register(registers::A, static_cast<uint8_t>(sub));
	set_flag(flag_registers::z, static_cast<uint8_t>(sub) == 0);
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::h, (a_val & 0xf) < (val & 0xf));
	set_flag(flag_registers::c, val > a_val);
}

void Cpu::sbc_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	uint8_t carry = get_flag(flag_registers::c);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	uint16_t sbc = a_val - val - carry;

	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::c, (sbc >> 8) != 0);
	set_register(registers::A, static_cast<uint8_t>(sbc));
	set_flag(flag_registers::z, static_cast<uint8_t>(sbc) == 0);
	set_flag(flag_registers::h, (a_val & 0xF) < (val & 0xF) + carry);
}

void Cpu::sbc_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint8_t a_val = get_register(registers::A);
	uint8_t carry = get_flag(flag_registers::c);
	uint16_t sbc = a_val - val - carry;
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::c, (sbc >> 8) != 0);
	set_flag(flag_registers::z, static_cast<uint8_t>(sbc) == 0);
	set_flag(flag_registers::h, (a_val & 0xF) < (val & 0xF) + carry);
	set_register(registers::A, static_cast<uint8_t>(sbc));
}

void Cpu::dec_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(registers::HL)) - 1;
		return;
	}
	mmap.write_u8(get_16bitregister(registers::HL), static_cast<uint8_t>(read_cache));
	get_flag(flag_registers::c) ? set_register(registers::F, 1U << flag_registers::c) : set_register(registers::F, 0);
	set_flag(flag_registers::z, static_cast<uint8_t>(read_cache) == 0);
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::h, (read_cache & 0x0F) == 0x0F);
}