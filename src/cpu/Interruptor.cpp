#include "Interruptor.hpp"
#include "Cpu.hpp"
#include <cstdint>

Interruptor::Interruptor(Cpu *cpu) : cpu(cpu) {}
Interruptor::~Interruptor() {}

void Interruptor::timer_tick(uint8_t cycle) {
	div_cycle += cycle;
	uint16_t div_cycles_max = cpu->cycle_speed(256);
	while (div_cycle >= div_cycles_max) {
		div_cycle -= div_cycles_max;
		timer_divider++;
	}

	if (timer_enable) {
		tima_cycle += cycle;
		uint16_t increment_freq = 0;
		switch (timer_clock_select) {
		case M_Cycles_256:
			increment_freq = cpu->cycle_speed(1024);
			break;
		case M_Cycles_4:
			increment_freq = cpu->cycle_speed(16);
			break;
		case M_Cycles_16:
			increment_freq = cpu->cycle_speed(64);
			break;
		case M_Cycles_64:
			increment_freq = cpu->cycle_speed(256);
			break;
		}

		while (tima_cycle >= increment_freq) {
			tima_cycle -= increment_freq;

			if (timer_counter == 0xFF) {
				timer_counter = timer_modulo;
				set_interrupt(InterruptType::Timer);
			} else
				timer_counter++;
		}
	}
}

void Interruptor::serial_tick(uint8_t cycle) {
	if (IsSetBit(serial_transfer_control, 7) && IsSetBit(serial_transfer_control, 0)) {
		serial_cycle += cycle;

		if (serial_count < 0) {
			serial_count = 0;
			serial_cycle = 0;
			return;
		}

		uint16_t serial_cycles_max = cpu->cycle_speed(512);

		if (serial_cycle >= serial_cycles_max) {
			if (serial_count > 7) {
				serial_transfer_control &= 0x7F;
				set_interrupt(Serial);
				serial_count -= 1;
				return;
			}

			serial_transfer_data <<= 1;
			serial_transfer_data |= 0x01;
			serial_cycle -= serial_cycles_max;
			serial_count += 1;
		}
	}
}

void Interruptor::check_cycles(uint16_t cycle, uint8_t state) {
	if (!interrupt_occured && delay_cycles > 0) {
		delay_cycles -= cycle;
		// interruptor.decrease_delay_cycles(t_cycle);
	}
	if (!interrupt_occured && state == StateReady && ime_cycles > 0) {
		ime_cycles -= cycle;
		if (ime_cycles <= 0) {
			ime_cycles = 0;
			process_interrupts = true;
		}
	}
}

bool Interruptor::handle_interrupt(uint8_t state) {
	interrupt_occured = false;
	if (!(state == StateReady && pending() != NoInterrupt && process_interrupts)) {
		return false;
	}
	interrupt_occured = true;
	uint8_t masked = interrupt_enable_register & interrupt;
	if (!masked) {
		printf("should not happen right?\n");
		return true;
	}
	if (cpu->is_halted()) {
		cpu->unhalt_cpu();
	}

	// Handle the interrupt, while taking the priority into account
	if (process_interrupts) {
		if (masked & InterruptType::Vblank) {
			delay_cycles = 0;
			process_interrupt(InterruptType::Vblank);
		} else if (masked & InterruptType::Stat)
			process_interrupt(InterruptType::Stat);
		else if (masked & InterruptType::Timer)
			process_interrupt(InterruptType::Timer);
		else if (masked & InterruptType::Serial)
			process_interrupt(InterruptType::Serial);
		else if (masked & InterruptType::Joypad)
			process_interrupt(InterruptType::Joypad);
	}
	return true;
}

void Interruptor::process_interrupt(InterruptType i) {
	cpu->process_interrupt(i);
	process_interrupts = false;
	interrupt &= ~i;
	#ifdef DEBUG_MODE
	printf("changing interrupt: %u\n", interrupt);
		#endif
}

void Interruptor::set_interrupt(InterruptType i) {
	interrupt |= static_cast<uint8_t>(i);
	#ifdef DEBUG_MODE
	printf("changing interrupt: %u\n", interrupt);
		#endif
}

void Interruptor::overwrite_interrupt(uint8_t val) {
	interrupt = val;
}

uint8_t Interruptor::get_interrupt() const {
	return interrupt;
}

bool Interruptor::get_ime() const {
	return process_interrupts;
}

InterruptType Interruptor::pending() const {
	uint8_t masked = interrupt_enable_register & interrupt;

	if ((masked & 0x1F) == 0) {
		return NoInterrupt;
	} else if ((masked & 0x01) && (delay_cycles <= 0)) {
		return Vblank;
	} else if (masked & 0x02) {
		return Stat;
	} else if (masked & 0x04) {
		return Timer;
	} else if (masked & 0x08) {
		return Serial;
	} else if (masked & 0x10) {
		return Joypad;
	}

	return NoInterrupt;
}

void Interruptor::set_interrupt_enable(uint8_t val) {
	interrupt_enable_register = val;
}

uint8_t Interruptor::get_interrupt_enable() {
	return interrupt_enable_register;
}

uint8_t Interruptor::get_timer_divider() {
	return timer_divider;
}

void Interruptor::reset_timer_divider() {
	timer_divider = 0;
}

uint8_t Interruptor::get_timer_counter() {
	return timer_counter;
}

void Interruptor::set_timer_counter(uint8_t val) {
	timer_counter = val;
}

uint8_t Interruptor::get_timer_module() {
	return timer_modulo;
}

void Interruptor::set_timer_modulo(uint8_t val) {
	timer_modulo = val;
}

uint8_t Interruptor::get_timer_control() {
	return (timer_enable << 2 | timer_clock_select << 0);
}

void Interruptor::set_timer_control(uint8_t val) {
	timer_enable = (val >> 2) & 1;
	if (!timer_enable) {
		tima_cycle = 0;
	}
	timer_clock_select = (val >> 0) & 3;
}

bool Interruptor::interrupt_ready() const {
	return (interrupt & interrupt_enable_register & 0x1F) != 0;
}

void Interruptor::enable_processing() {
	process_interrupts = true;
}

void Interruptor::disable_processing() {
	process_interrupts = false;
}

int16_t Interruptor::get_ime_cycles() {
	return ime_cycles;
}

void Interruptor::reset_ime_cycles() {
	ime_cycles = 0;
	process_interrupts = true;
}

void Interruptor::set_ime_cycles(int16_t cycle) {
	ime_cycles = cycle;
}

void Interruptor::check_halt_bug() {
	if (cpu->get_mmap().is_cgb_rom() && !process_interrupts && (interrupt & interrupt_enable_register & 0x1F)) {
		halt_bug_triggered = true;
	}
}

int16_t Interruptor::get_delay_cycles() {
	return delay_cycles;
}

void Interruptor::set_delay_cycles(uint16_t cycle) {
	delay_cycles = cycle;
}

void Interruptor::reset_interrupt() {
	interrupt_occured = false;
}
