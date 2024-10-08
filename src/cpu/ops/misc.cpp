#include "Cpu.hpp"
#include "OpcodeTiming.hpp"

void Cpu::nop() {}

void Cpu::daa() {
	uint8_t a_val = get_register(registers::A);
	if (get_flag(flag_registers::n)) {
		if (get_flag(flag_registers::c))
			a_val -= 0x60;
		if (get_flag(flag_registers::h))
			a_val -= 0x6;
	} else {
		if (get_flag(flag_registers::c) || a_val > 0x99) {
			a_val += 0x60;
			set_flag(flag_registers::c, 1);
		}
		if (get_flag(flag_registers::h) || ((a_val & 0x0F) > 0x09))
			a_val += 0x6;
	}
	set_flag(flag_registers::z, a_val == 0);
	set_flag(flag_registers::h, 0);
	set_register(registers::A, a_val);
}

void Cpu::cpl() {
	set_register(registers::A, ~get_register(registers::A));
	set_flag(flag_registers::n, 1);
	set_flag(flag_registers::h, 1);
}

void Cpu::scf() {
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	set_flag(flag_registers::c, 1);
}

void Cpu::ccf() {
	set_flag(flag_registers::n, 0);
	set_flag(flag_registers::h, 0);
	set_flag(flag_registers::c, !get_flag(flag_registers::c));
}

void Cpu::stop() {
	pc += 1;
	if (mmap.is_cgb_rom()) {
		uint8_t key1 = mmap.read_io_registers(0xFF4D);

		if ((key1 & mask0)) {
			cgb_speed = !cgb_speed;

			if (cgb_speed) {
				speed_multiplier = 1;
				mmap.write_io_registers(0xFF4D, 0x80);
			} else {
				speed_multiplier = 0;
				mmap.write_io_registers(0xFF4D, 0x00);
			}
		}
	}
}

void Cpu::halt() {
	if (interruptor.get_ime_cycles() > 0) {
		interruptor.reset_ime_cycles();
		pc -= 1;
	} else {
		halted = true;
		interruptor.check_halt_bug();
	}
}

void Cpu::di() {
	interruptor.reset_ime_cycles();
	interruptor.disable_processing();
}

void Cpu::ei() {
	int16_t cycles = machine_cycles[instruction_list::Unprefixed][0xFB] * cycle_speed(4);
	interruptor.set_ime_cycles(cycles + 1);
}