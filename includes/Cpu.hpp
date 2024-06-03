#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include "Decoder.hpp"
#include "MemoryMap.hpp"
#include <iostream>
#include "PixelProcessingUnit.hpp"
#include <bitset>

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

	PixelProcessingUnit ppu;

	// TODO check all functions are actually used

	//  Block 0
	void nop();

	void ld_r16_imm16(Operand op_r);
	void ld_r16_a(Operand op_r);
	void ld_a_r16(Operand op_s);
	void ld_imm16_sp();

	void inc_r16(Operand op_r);
	void dec_r16(Operand op_r);
	void add_hl_r16(Operand op_s);

	void inc_r8(Operand op_r);
	void dec_r8(Operand op_r);

	void ld_r8_imm8(Operand op_r);

	void rlca();
	void rrca();
	void rla();
	void rra();
	void daa();
	void cpl();
	void scf();
	void ccf();

	void jr_imm8();
	void jr_cond_imm8(Condition c);

	void stop();

	// block1 register to register loads
	void ld_r8_r8(Operand op_r, Operand op_s);
	void halt();

	// block2 8-bit arithmetic
	void add_a_r8(Operand op_s);
	void adc_a_r8(Operand op_s);
	void sub_a_r8(Operand op_s);
	void sbc_a_r8(Operand op_s);
	void and_a_r8(Operand op_s);
	void xor_a_r8(Operand op_s);
	void or_a_r8(Operand op_s);
	void cp_a_r8(Operand op_s);

	// block3
	void add_a_imm8();
	void adc_a_imm8();
	void sub_a_imm8();
	void sbc_a_imm8();
	void and_a_imm8();
	void xor_a_imm8();
	void or_a_imm8();
	void cp_a_imm8();

	void ret_cond(Condition c);
	void ret();
	void reti();
	void jp_cond_imm16(Condition c);
	void jp_imm16();
	void jp_hl();
	void call_cond_imm16(Condition c);
	void call_imm16();
	void rst_tg3(Instruction in);

	void pop_r16stk(Operand op_r);
	void push_r16stk(Operand op_s);

	void ldh_imm8_a();
	void ld_imm16_a();
	void ldh_a_imm8();
	void ld_a_imm16();

	void add_sp_imm8();
	void ld_hl_sp_imm8();
	void ld_sp_hl();

	void di();
	void ei();

	// CB prefix
	void prefix(Instruction in, uint8_t opcode);
	void rlc_r8(uint8_t opcode, Operand op_r);
	void rrc_r8(uint8_t opcode, Operand op_r);
	void rl_r8(uint8_t opcode, Operand op_r);
	void rr_r8(uint8_t opcode, Operand op_r);
	void sla_r8(uint8_t opcode, Operand op_r);
	void sra_r8(uint8_t opcode, Operand op_r);
	void swap_r8(uint8_t opcode, Operand op_r);
	void srl_r8(uint8_t opcode, Operand op_r);

	uint8_t get_rlc(uint8_t val, bool reset = false);
	uint8_t get_rrc(uint8_t val, bool reset = false);
	uint8_t get_rr(uint8_t val, bool reset = false);
	uint8_t get_rl(uint8_t val, bool reset = false);

	void bit_b3_r8(uint8_t opcode, Operand op_s);
	void res_b3_r8(uint8_t opcode, Operand op_s);
	void set_b3_r8(uint8_t opcode, Operand op_s);

	void unimplemented(uint8_t opcode);

	void lockup(); //$D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD

	void ld_i_hl_a();
	void ld_d_hl_a();
	void ld_i_a_hl();
	void ld_d_a_hl();
	void ld_hl_imm8();
	void ld_c_a();
	void ld_a_c();

	void ld(uint8_t opcode, std::vector<Operand> operands); // to go to the correct ld function
	void inc(uint8_t opcode, Operand op_r);				 // to go to the correct inc function
	void dec(uint8_t opcode, Operand op_r);				 // to go to the correct dec function
	void add(uint8_t opcode, Operand op_s);
	void jr(uint8_t opcode);
	void adc(uint8_t opcode, Operand op_s);
	void sub(uint8_t opcode, Operand op_s);
	void sbc(uint8_t opcode, Operand op_s);
	void and_(uint8_t opcode, Operand op_s);
	void xor_(uint8_t opcode, Operand op_s);
	void or_(uint8_t opcode, Operand op_s);
	void cp_(uint8_t opcode, Operand op_s);
	void ret(uint8_t opcode);
	void jp(uint8_t opcode);
	void call(uint8_t opcode);
	void ldh(uint8_t opcode);

	// TODO template these
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

	std::tuple<uint8_t, Instruction, bool> get_instruction();

	void debug_print(Instruction in, uint8_t opcode);

public:
	Cpu(Decoder dec, const std::string path);
	~Cpu();

	uint8_t get_register(Registers reg) const;
	uint16_t get_16bitregister(Registers reg) const;
	uint8_t get_flag(uint8_t flag) const;

	void set_16bitregister(Registers reg, uint16_t val);
	void set_register(Registers reg, uint8_t val);
	void set_flag(uint8_t flag, uint8_t val);

	void execute_instruction(Instruction in, uint8_t opcode, bool is_prefix_ins);
	void tick();
	void event_handler();
	void handle_input(SDL_Event &e);
	inline bool status() {return ppu.status();}
	inline void close() {ppu.close();}
};

#endif