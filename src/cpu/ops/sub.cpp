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