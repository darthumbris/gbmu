#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include <math.h>
#include <iostream>
#include <bitset>
#include "MemoryMap.hpp"
#include "Operand.hpp"
#include "Decoder.hpp"
#include "PixelProcessingUnit.hpp"

using namespace Dict;

enum FlagRegisters
{
	c = 4,
	h = 5,
	n = 6,
	z = 7
};

enum Condition
{
	NotZeroFlag,
	ZeroFlag,
	NotCarryFlag,
	CarryFlag,
};

 enum InterruptType {
	Vblank = 1 << 0, //0x40 Vblank interrupt
	Stat = 1 << 1, // 0x48 STAT interrupt
	Timer = 1 << 2, //0x50 Timer interrupt
	Serial = 1 << 3, //0x58 Serial interrupt
	Joypad  = 1 << 4 //0x60 Joypad interrupt
 };

class Cpu
{
private:
	/*
	 * registers (
		AF Accumaltor & Flag register,
		BC general purpose register, (B High byte, C Low byte)
		DE general purpose register,
		HL general purpose register and instructions for iterative code
		SP stack pointer
		PC program counter
		)
	 *
	 * flag register (z zero flag, h half carry flag, n subtraction flag, c carry flag)
	 *
	 * The registers are made uint16_t because the gameboy uses 16-bit registers
	 */
	std::array<uint8_t, 8> u8_registers;

	uint16_t sp; //stack pointer
	uint16_t pc; // Program counter
	Decoder decoder;
	MemoryMap mmap;
	PixelProcessingUnit ppu;
	bool halted = false;
	uint8_t interrupt_enable_register = 0; //0xFFFF
	uint8_t interrupt = 0; //0xFF0F
	uint16_t m_cycle;
	uint16_t t_cycle;
	uint16_t d_cycle=0;
	bool process_interrupts = false;
	uint8_t opcode = 0;

	uint8_t timer_divider = 0;

	typedef void (Cpu::*OpsFn)(void);

	std::array<OpsFn, 256> unprefixed_instructions;
	std::array<OpsFn, 256> prefixed_instructions;
	void set_instructions();

	void prefix();

	template<uint8_t opcode>
	void unimplemented() {std::cout << "unimplemented opcode: 0x" << std::setfill('0') << std::setw(2) << std::hex << opcode << std::dec << std::endl;}

	#include "ops/ops_add.tcc"
	#include "ops/ops_alu.tcc"
	#include "ops/ops_bit.tcc"
	#include "ops/ops_call.tcc"
	#include "ops/ops_dec.tcc"
	#include "ops/ops_inc.tcc"
	#include "ops/ops_jumps.tcc"
	#include "ops/ops_loads.tcc"
	#include "ops/ops_misc.tcc"
	#include "ops/ops_reset.tcc"
	#include "ops/ops_ret.tcc"
	#include "ops/ops_rotate_shift.tcc"
	#include "ops/ops_sub.tcc"

	void lockup(); //$D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD

	template <typename IntegerType1, typename IntegerType2>
	bool half_carry_flag_set(IntegerType1 val1, IntegerType2 val2)
	{
		return (((val1 & 0xF) + (val2 & 0xF)) > 0xF);
	}

	template <typename IntegerType1, typename IntegerType2>
	bool carry_flag_set(IntegerType1 val1, IntegerType2 val2)
	{
		return (((val1 & 0xFF) + (val2 & 0xFF)) > 0xFF);
	}
	void execute_instruction();

	void debug_print(bool prefix);

	inline void set_cycle(uint8_t c) {m_cycle += c; t_cycle += c * 4;}

	void handle_interrupt();
	void process_interrupt(InterruptType i);

public:
	uint64_t debug_count;
	Cpu(Decoder dec, const std::string path);
	~Cpu();

	INLINE_FN uint8_t get_register(Registers reg) const;
	INLINE_FN uint16_t get_16bitregister(Registers reg) const;
	INLINE_FN uint8_t get_flag(uint8_t flag) const;

	INLINE_FN void set_16bitregister(Registers reg, uint16_t val);
	INLINE_FN void set_register(Registers reg, uint8_t val);
	INLINE_FN void set_flag(uint8_t flag, uint8_t val);

	inline PixelProcessingUnit& get_ppu() {return ppu;}
	inline MemoryMap& get_mmap() {return mmap;}

	
	void tick();
	void event_handler();
	void handle_input(SDL_Event &e);
	inline bool status() {return ppu.status();}
	inline void set_status(bool val) {ppu.set_status(val);}
	inline void close() {ppu.close();}
	inline void set_interrupt(InterruptType i) {
		// std::cout << "requesting interrupt if: " << (uint16_t)i << std::endl;
		interrupt |= static_cast<uint8_t>(i);}
	inline void overwrite_interrupt(uint8_t val) {
		// std::cout << "settting if: " << (uint16_t)val << std::endl;
		interrupt = val;}
	inline void set_interrupt_enable(uint8_t val) {
		// std::cout << "settting ie: " << (uint16_t)val << std::endl;
		interrupt_enable_register =val;}
	inline uint8_t get_interrupt_enable() {return interrupt_enable_register;}
	inline uint8_t get_timer_divider() {return timer_divider;}
	inline void reset_timer_divider() {timer_divider = 0;}
};

#endif