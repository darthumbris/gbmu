#include "Cpu.hpp"

void Cpu::sub_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	uint16_t sub = a_val - val;
	set_flag(FlagRegisters::z, sub == 0);
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
	set_flag(FlagRegisters::c, val > a_val);
	set_register(Registers::A, static_cast<uint8_t>(sub));
}

void Cpu::sub_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint8_t a_val = get_register(Registers::A);
	uint16_t sub = a_val - val;
	set_register(Registers::A, static_cast<uint8_t>(sub));
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sub) == 0);
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
	set_flag(FlagRegisters::c, val > a_val);
}

void Cpu::sbc_a_imm8() {
	uint8_t val = mmap.read_u8(pc);
	uint8_t carry = get_flag(FlagRegisters::c);
	pc += 1;
	uint8_t a_val = get_register(Registers::A);
	uint16_t sbc = a_val - val - carry;

	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::c, (sbc >> 8) != 0);
	set_register(Registers::A, static_cast<uint8_t>(sbc));
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sbc) == 0);
	set_flag(FlagRegisters::h, (a_val & 0xF) < (val & 0xF) + carry);
}

void Cpu::sbc_a_r8_hl() {
	uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
	uint8_t a_val = get_register(Registers::A);
	uint8_t carry = get_flag(FlagRegisters::c);
	uint16_t sbc = a_val - val - carry;
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::c, (sbc >> 8) != 0);
	set_flag(FlagRegisters::z, static_cast<uint8_t>(sbc) == 0);
	set_flag(FlagRegisters::h, (a_val & 0xF) < (val & 0xF) + carry);
	set_register(Registers::A, static_cast<uint8_t>(sbc));
}

void Cpu::dec_r8_hl() {
	if (accurate_opcode_state == StateReadingWord) {
		read_cache = mmap.read_u8(get_16bitregister(Registers::HL)) - 1;
		return;
	}
	mmap.write_u8(get_16bitregister(Registers::HL), static_cast<uint8_t>(read_cache));
	uint8_t before_flag = get_register(Registers::F);
	get_flag(FlagRegisters::c) ? set_register(Registers::F, 1U << FlagRegisters::c) : set_register(Registers::F, 0);
	set_flag(FlagRegisters::z, static_cast<uint8_t>(read_cache) == 0);
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::h, (read_cache & 0x0F) == 0x0F);
}