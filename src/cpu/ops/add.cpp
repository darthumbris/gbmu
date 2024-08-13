#include "Cpu.hpp"

void Cpu::add_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	uint16_t sum = a_val + val;
	set_register(Registers::A, static_cast<uint8_t>(sum));
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
	set_flag(FlagRegisters::c, (sum >> 8) != 0);
}

void Cpu::add_a_r8_hl() {
	uint8_t a_val = get_register(Registers::A);
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint16_t sum = a_val + val;
	set_register(Registers::A, static_cast<uint8_t>(sum));
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
	set_flag(FlagRegisters::c, (sum >> 8) != 0);
}

void Cpu::add_sp_imm8() {
	int8_t e8 = mmap.read_u8(pc);
	pc += 1;
	uint16_t sp_val = sp;
	set_flag(FlagRegisters::z, 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, ((e8 & 0xf) + (sp_val & 0xf)) > 0xf);
	set_flag(FlagRegisters::c, ((e8 & 0xff) + (sp_val & 0xff)) > 0xff);
	sp = static_cast<uint16_t>(e8 + sp_val);
}

void Cpu::adc_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	uint8_t carry = get_flag(FlagRegisters::c);
	uint16_t sum = a_val + val + carry;
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
	set_flag(FlagRegisters::c, (sum >> 8) != 0);
	set_register(Registers::A, sum);
}

void Cpu::adc_a_r8_hl() {
	uint8_t a_val = get_register(Registers::A);
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint8_t carry = get_flag(FlagRegisters::c);
	uint16_t sum = a_val + val + carry;
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
	set_flag(FlagRegisters::c, (sum >> 8) != 0);
	set_register(Registers::A, sum);
}

void Cpu::inc_r8_hl() {
	if (accurate_opcode_state == StateReadingWord) {
		uint8_t val;
		uint16_t res;
		val = mmap.read_u8(get_16bitregister(Registers::HL));
		res = val + 1;
		read_cache = res;
		return;
	}
	uint8_t before_flag = get_register(Registers::F);
	mmap.write_u8(get_16bitregister(Registers::HL), static_cast<uint8_t>(read_cache));
	get_flag(FlagRegisters::c) ? set_register(Registers::F, 1U << FlagRegisters::c) : set_register(Registers::F, 0);
	set_flag(FlagRegisters::z, static_cast<uint8_t>(read_cache) == 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, (read_cache & 0x0F) == 0x00);
}