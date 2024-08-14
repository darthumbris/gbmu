#ifndef INTERRUPTOR_HPP
#define INTERRUPTOR_HPP

#include <cstdint>
#include <fstream>

class Cpu;
enum class instruction_state;

enum class interrupt_type {
	NoInterrupt = 0,
	Vblank = 1 << 0, // 0x40 Vblank interrupt
	Stat = 1 << 1,   // 0x48 STAT interrupt
	Timer = 1 << 2,  // 0x50 Timer interrupt
	Serial = 1 << 3, // 0x58 Serial interrupt
	Joypad = 1 << 4  // 0x60 Joypad interrupt
};

enum clock_type {
	M_Cycles_256,
	M_Cycles_4,
	M_Cycles_16,
	M_Cycles_64,
};

class Interruptor {
private:
	uint8_t interrupt_enable_register = 0; // 0xFFFF
	uint8_t interrupt = 0;                 // 0xFF0F
	bool process_interrupts = false;       // Interrupt Master Enable (IME)
	bool interrupt_occured = false;
	int16_t ime_cycles = 0;
	int16_t delay_cycles = 0;
	bool halt_bug_triggered = false;

	uint8_t serial_transfer_data;    // 0xFF01
	uint8_t serial_transfer_control; // 0xFF02
	uint8_t timer_divider = 0;       // 0xFF04
	uint8_t timer_counter = 0;       // 0xFF05
	uint8_t timer_modulo = 0;        // 0xFF06
	bool timer_enable = false;       // 0xFF07
	uint8_t timer_clock_select = 0;  // 0xFF07

	uint16_t div_cycle = 0;  // for 0xFF04
	uint16_t tima_cycle = 0; // For 0xFF05

	uint16_t serial_cycle = 0; // for 0xFF01 and 0xFF02
	int32_t serial_count = 0;  // for 0xFF02

	uint16_t input_cycles = 0;

	// TODO check the interrupt delay cycles thing

	Cpu *cpu;

public:
	Interruptor(Cpu *cpu);
	~Interruptor();

	void timer_tick(uint8_t cycle);
	void serial_tick(uint8_t cycle);
	void input_tick(uint8_t cycle);
	bool handle_interrupt(instruction_state state);
	void process_interrupt(interrupt_type i);
	void enable_processing();
	void disable_processing();

	void set_serial_transfer_control(uint8_t val);
	void set_serial_transfer_data(uint8_t val);
	uint8_t get_serial_transfer_control();
	uint8_t get_serial_transfer_data();
	void set_interrupt(interrupt_type i);
	void overwrite_interrupt(uint8_t val);
	uint8_t get_interrupt() const;
	interrupt_type pending() const;
	void set_interrupt_enable(uint8_t val);
	uint8_t get_interrupt_enable();
	uint8_t get_timer_divider();
	void reset_timer_divider();
	uint8_t get_timer_counter();
	void set_timer_counter(uint8_t val);
	uint8_t get_timer_module();
	void set_timer_modulo(uint8_t val);
	uint8_t get_timer_control();
	void set_timer_control(uint8_t val);
	bool interrupt_ready() const;
	bool get_ime() const;
	void check_cycles(uint16_t cycle, instruction_state state);

	int16_t get_ime_cycles();
	int16_t get_delay_cycles();
	void set_delay_cycles(uint16_t cycle);
	void reset_ime_cycles();
	void set_ime_cycles(int16_t cycle);
	void check_halt_bug();
	void reset_interrupt();

	void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f);
};

#endif