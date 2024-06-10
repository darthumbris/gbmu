#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include "Decoder.hpp"
#include "MemoryMap.hpp"
#include <iostream>
#include <bitset>
#include "Operand.hpp"

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
	// std::array<uint16_t, 5> registers; //(all low and high register values and the StackPointer (the combined (so 8 uint8_t > 4 uint16_t)))
	std::array<uint8_t, 8> u8_registers;
	uint32_t debug_count;

	uint16_t sp; //stack pointer
	uint16_t pc; // Program counter
	Decoder decoder;
	MemoryMap mmap;
	bool halted = false;	 // TODO check if this can be handled by the interrupt in the memorymap
	bool interrupts = false; // TODO check if can be done with interrupt in the memorymap?
	uint16_t m_cycle;
	uint16_t t_cycle;

	typedef void (Cpu::*OpsFn)(void);

	std::array<OpsFn, 256> unprefixed_instructions;
	std::array<OpsFn, 256> prefixed_instructions;
	void set_instructions();

	// CB prefix
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

	void debug_print(uint8_t opcode, bool prefix);

	inline void set_cycle(uint8_t c) {m_cycle = c; t_cycle = c * 4;} 

public:
	Cpu(Decoder dec, const std::string path);
	~Cpu();

	INLINE_FN uint8_t get_register(Registers reg) const;
	INLINE_FN uint16_t get_16bitregister(Registers reg) const;
	INLINE_FN uint8_t get_flag(uint8_t flag) const;

	INLINE_FN void set_16bitregister(Registers reg, uint16_t val);
	INLINE_FN void set_register(Registers reg, uint8_t val);
	INLINE_FN void set_flag(uint8_t flag, uint8_t val);

	
	void tick();
	void event_handler();
	void handle_input(SDL_Event &e);
	inline bool status() {return mmap.status();}
	inline void set_status(bool val) {mmap.set_status(val);}
	inline void close() {mmap.close();}
};

#endif