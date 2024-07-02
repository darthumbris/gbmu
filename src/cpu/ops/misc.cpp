#include "Cpu.hpp"

void Cpu::nop() {
	set_cycle(1);
}

void Cpu::daa() {
	uint8_t a_val = get_register(Registers::A);
	if (get_flag(FlagRegisters::n)) {
		if (get_flag(FlagRegisters::c))
			a_val -= 0x60;
		if (get_flag(FlagRegisters::h))
			a_val -= 0x6;
	} else {
		if (get_flag(FlagRegisters::c) || a_val > 0x99) {
			a_val += 0x60;
			set_flag(FlagRegisters::c, 1);
		}
		if (get_flag(FlagRegisters::h) || ((a_val & 0x0F) > 0x09))
			a_val += 0x6;
	}
	set_flag(FlagRegisters::z, a_val == 0);
	set_flag(FlagRegisters::h, 0);
	set_register(Registers::A, a_val);
	set_cycle(1);
}

void Cpu::cpl() {
	set_register(Registers::A, ~get_register(Registers::A));
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::h, 1);
	set_cycle(1);
}

void Cpu::scf() {
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::c, 1);
	set_cycle(1);
}

void Cpu::ccf() {
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::c, !get_flag(FlagRegisters::c));
	set_cycle(1);
}

void Cpu::stop() {
	pc += 1;
	//TODO for CGB might need to check 0xFF4D
	set_cycle(1);
}

void Cpu::halt() {
	halted = true;
	set_cycle(1);
}

void Cpu::di() {
	process_interrupts = false;
	set_cycle(1);
}

void Cpu::ei() {
	process_interrupts = true;
	set_cycle(1);
}