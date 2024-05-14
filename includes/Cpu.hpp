#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <array>
#include "Decoder.hpp"

using namespace Dict;

enum LowRegisters
{
	L_C = 1,
	L_E = 3,
	L_L = 5,
	L_F = 7,
};

enum HighRegisters
{
	H_B = 0,
	H_D = 2,
	H_H = 4,
	H_A = 6,
};

enum Registers
{
	B = HighRegisters::H_B,
	C = LowRegisters::L_C,
	D = HighRegisters::H_D,
	E = LowRegisters::L_E,
	H = HighRegisters::H_H,
	L = LowRegisters::L_L,
	A = HighRegisters::H_A,
	F = LowRegisters::L_F,
	BC = 8,
	DE = 9,
	HL = 10,
	AF = 11,
	SP = 12,
};

enum FlagRegisters
{
	c = 4,
	h = 5,
	n = 6,
	z = 7
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
	std::array<uint16_t, 5> registers; //(all low and high register values and the StackPointer (the combined (so 8 uint8_t > 4 uint16_t)))

	uint16_t pc; // Program counter
	Decoder decoder;

	// Block 0
	void nop();

	void ld_r16_imm16();
	void ld_r16_a();
	void ld_a_r16();
	void ld_imm16_sp();

	void inc_r16();
	void dec_r16();
	void add_hl_r16();

	void inc_r8();
	void dec_r8();

	void ld_r8_imm8();

	void rlca();
	void rrca();
	void rla();
	void rra();
	void daa();
	void cpl();
	void scf();
	void ccf();

	void jr_imm8();
	void jr_cond_imm8();

	void stop();

	// block1 register to register loads
	void ld_r8_r8();
	void halt();

	// block2 8-bit arithmetic
	void add_a_r8();
	void adc_a_r8();
	void sub_a_r8();
	void sbc_a_r8();
	void and_a_r8();
	void xor_a_r8();
	void or_a_r8();
	void cp_a_r8();

	// block3
	void add_a_imm8();
	void adc_a_imm8();
	void sub_a_imm8();
	void sbc_a_imm8();
	void and_a_imm8();
	void xor_a_imm8();
	void or_a_imm8();
	void cp_a_imm8();

	void ret_cond();
	void ret();
	void reti();
	void jp_cond_imm16();
	void jp_imm16();
	void jp_hl();
	void call_cond_imm16();
	void call_imm16();
	void rst_tg3();

	void pop_r16stk();
	void push_r16stk();

	// void prefix();

	void ldh_c_a();
	void ldh_imm8_a();
	void ld_imm16_a();
	void ldh_a_c();
	void ldh_a_imm8();
	void ld_a_imm16();

	void add_sp_imm8();
	void ld_hl_sp_imm8();
	void ld_sp_hl();

	void di();
	void ei();

	// CB prefix
	void prefix(Instruction in, uint16_t opcode);
	void rlc_r8();
	void rrc_r8();
	void rl_r8();
	void rr_r8();
	void sla_r8();
	void sra_r8();
	void swap_r8();
	void srl_r8();

	void bit_b3_r8();
	void res_b3_r8();
	void set_b3_r8();

	void unimplemented();

	void lockup(); //$D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD

	void ld_increment_hl();
	void ld_decrement_hl();
	void ld_hl_imm8();
	void ld_c_a();
	void ld_a_c();

	void ld(uint16_t opcode);  // to go to the correct ld function
	void inc(uint16_t opcode); // to go to the correct inc function
	void dec(uint16_t opcode); // to go to the correct dec function

public:
	Cpu(Decoder dec);
	~Cpu();

	uint16_t get_register(Registers reg) const;
	uint8_t get_register_bit(Registers reg, uint8_t bit_loc) const;
	uint8_t get_flag(uint8_t flag) const;

	void set_register(Registers reg, uint16_t val);
	void set_register_bit(Registers reg, uint8_t bit_loc, uint8_t val);
	void set_flag(uint8_t flag, uint8_t val);

	void execute_instruction(Instruction in, uint16_t opcode, bool is_prefix_ins);
	void tick();
};

#endif