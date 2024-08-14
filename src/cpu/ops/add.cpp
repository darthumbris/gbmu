#include "Cpu.hpp"

void Cpu::add_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	uint16_t sum = a_val + val;
	set_register(registers::A, static_cast<uint8_t>(sum));
	set_flag(flag_registers::z, static_cast<uint8_t>(sum) == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
	set_flag(flag_registers::c, (sum >> 8) != 0);
}

void Cpu::add_a_r8_hl() {
	uint8_t a_val = get_register(registers::A);
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint16_t sum = a_val + val;
	set_register(registers::A, static_cast<uint8_t>(sum));
	set_flag(flag_registers::z, static_cast<uint8_t>(sum) == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
	set_flag(flag_registers::c, (sum >> 8) != 0);
}

void Cpu::add_sp_imm8() {
	int8_t e8 = mmap.read_u8(pc);
	pc += 1;
	uint16_t sp_val = sp;
	set_flag(flag_registers::z, 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, ((e8 & 0xf) + (sp_val & 0xf)) > 0xf);
	set_flag(flag_registers::c, ((e8 & 0xff) + (sp_val & 0xff)) > 0xff);
	sp = static_cast<uint16_t>(e8 + sp_val);
}

void Cpu::adc_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(registers::A);
	uint8_t carry = get_flag(flag_registers::c);
	uint16_t sum = a_val + val + carry;
	set_flag(flag_registers::z, static_cast<uint8_t>(sum) == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
	set_flag(flag_registers::c, (sum >> 8) != 0);
	set_register(registers::A, sum);
}

void Cpu::adc_a_r8_hl() {
	uint8_t a_val = get_register(registers::A);
	uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
	uint8_t carry = get_flag(flag_registers::c);
	uint16_t sum = a_val + val + carry;
	set_flag(flag_registers::z, static_cast<uint8_t>(sum) == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
	set_flag(flag_registers::c, (sum >> 8) != 0);
	set_register(registers::A, sum);
}

void Cpu::inc_r8_hl() {
	if (accurate_opcode_state == instruction_state::ReadingWord) {
		uint8_t val;
		uint16_t res;
		val = mmap.read_u8(get_16bitregister(registers::HL));
		res = val + 1;
		read_cache = res;
		return;
	}
	uint8_t before_flag = get_register(registers::F);
	mmap.write_u8(get_16bitregister(registers::HL), static_cast<uint8_t>(read_cache));
	get_flag(flag_registers::c) ? set_register(registers::F, 1U << flag_registers::c) : set_register(registers::F, 0);
	set_flag(flag_registers::z, static_cast<uint8_t>(read_cache) == 0);
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, (read_cache & 0x0F) == 0x00);
}