#include "Cpu.hpp"
#include <cstdint>

void Cpu::nop() {
	//set_cycle(1);
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
	//set_cycle(1);
}

void Cpu::cpl() {
	set_register(Registers::A, ~get_register(Registers::A));
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::h, 1);
	//set_cycle(1);
}

void Cpu::scf() {
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::c, 1);
	//set_cycle(1);
}

void Cpu::ccf() {
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::c, !get_flag(FlagRegisters::c));
	//set_cycle(1);
}

void Cpu::stop() {
	pc += 1;
	if (mmap.is_cgb_rom())
    {
        uint8_t key1 = mmap.read_io_registers(0xFF4D);

        if (IsSetBit(key1, 0))
        {
            cgb_speed = !cgb_speed;

            if (cgb_speed)
            {
                speed_multiplier = 1;
				mmap.write_io_registers(0xFF4D, 0x80);
            }
            else
            {
                speed_multiplier = 0;
				mmap.write_io_registers(0xFF4D, 0x00);
            }
        }
    }
	//set_cycle(1);
}

void Cpu::halt() {
	// halted = true;

	if (interruptor.get_ime_cycles() > 0)
    {
        interruptor.reset_ime_cycles();
        pc -= 1;
	}
    else
    {
		halted = true;
		interruptor.check_halt_bug();
    }

	//set_cycle(1);
}

void Cpu::di() {
	interruptor.reset_ime_cycles();
	interruptor.disable_processing();
	//set_cycle(1);
}

void Cpu::ei() {
	// interruptor.enable_processing();
	int16_t cycles = UnprefixedMachineCycles[0xFB] * cycle_speed(4);
	interruptor.set_ime_cycles(cycles + 1);
	//set_cycle(1);
}