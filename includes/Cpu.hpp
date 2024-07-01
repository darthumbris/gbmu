#ifndef CPU_HPP
#define CPU_HPP

#include "Decoder.hpp"
#include "MemoryMap.hpp"
#include "Operand.hpp"
#include "PixelProcessingUnit.hpp"
#include <array>
#include <cstdint>
#include <math.h>

using namespace Dict;

enum FlagRegisters { c = 4, h = 5, n = 6, z = 7 };

enum Condition {
	NotZeroFlag,
	ZeroFlag,
	NotCarryFlag,
	CarryFlag,
};

enum InterruptType {
	Vblank = 1 << 0, // 0x40 Vblank interrupt
	Stat = 1 << 1,   // 0x48 STAT interrupt
	Timer = 1 << 2,  // 0x50 Timer interrupt //TODO check when this needs to be done
	Serial = 1 << 3, // 0x58 Serial interrupt //TODO check when this needs to be done
	Joypad = 1 << 4  // 0x60 Joypad interrupt
};

// TODO handle CGB freq etc

class Cpu {
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

	uint16_t sp; // stack pointer
	uint16_t pc; // Program counter
	Decoder decoder;
	MemoryMap mmap;
	PixelProcessingUnit ppu;
	bool halted = false;
	bool locked = false;
	uint8_t interrupt_enable_register = 0; // 0xFFFF
	uint8_t interrupt = 0;                 // 0xFF0F
	uint16_t m_cycle;
	uint16_t t_cycle;
	uint16_t d_cycle = 0;
	bool process_interrupts = false;
	uint8_t opcode = 0;

	uint8_t timer_divider = 0;      // 0xFF04
	uint8_t timer_counter = 0;      // 0xFF05
	uint8_t timer_modulo = 0;       // 0xFF06
	bool timer_enable = false;      // 0xFF07
	uint8_t timer_clock_select = 0; // 0xFF07
	std::string rom_path;

	using OpsFn = void (Cpu::*)();

	std::array<OpsFn, 256> unprefixed_instructions;
	std::array<OpsFn, 256> prefixed_instructions;
	void set_instructions();

	void prefix();

	template <uint8_t op>
	void unimplemented() {
		printf("Unimplemented opcode: %#04x\n", op);
	}

	//$D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD
	template <uint8_t op>
	void lockup() {
		locked = true;
		printf("Illegal instruction. Hard-Locks the Cpu. opcode: %#04x\n", op);
	}

	inline bool zero() const {
		return (u8_registers[Registers::F] >> 7) == 0b1;
	}
	inline bool carry() const {
		return ((u8_registers[Registers::F] >> 6) & 1) == 1;
	}
	inline bool half_carry() const {
		return ((u8_registers[Registers::F] >> 5) & 1) == 1;
	}
	inline bool subtraction() const {
		return ((u8_registers[Registers::F] >> 4) & 1) == 1;
	}

	void add_a_imm8();
	void add_sp_imm8();
	void adc_a_imm8();

	template <Registers src>
	void add_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		uint16_t sum = a_val + val;
		set_register(Registers::A, static_cast<uint8_t>(sum));
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
		set_flag(FlagRegisters::c, (sum >> 8) != 0);
	}

	template <Registers src>
	void add_hl_r16() {
		uint16_t val = get_16bitregister(src);
		uint16_t hl_val = get_16bitregister(Registers::HL);
		set_16bitregister(Registers::HL, static_cast<uint16_t>(val + hl_val));
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, (val & 0xfff) + (hl_val & 0xfff) > 0xfff);
		set_flag(FlagRegisters::c, (val + hl_val) >> 16);
		set_cycle(2);
	}

	template <Registers src>
	void adc_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		uint8_t carry = get_flag(FlagRegisters::c);
		uint16_t sum = a_val + val + carry;
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
		set_flag(FlagRegisters::c, (sum >> 8) != 0);
		set_register(Registers::A, sum);
	}

	void xor_a_imm8();
	void or_a_imm8();
	void cp_a_imm8();
	void and_a_imm8();

	template <Registers src>
	void and_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		set_register(Registers::A, a_val & val);
		set_flag(FlagRegisters::z, (a_val & val) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 1);
		set_flag(FlagRegisters::c, 0);
	}

	template <Registers src>
	void xor_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		a_val ^= val;
		set_register(Registers::A, a_val);
		set_register(Registers::F, 0);
		set_flag(FlagRegisters::z, a_val == 0);
	}

	template <Registers src>
	void or_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		set_register(Registers::A, a_val | val);
		set_register(Registers::F, 0);
		set_flag(FlagRegisters::z, (a_val | val) == 0);
	}

	template <Registers src>
	void cp_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		set_flag(FlagRegisters::z, (a_val == val));
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::c, a_val < val);
		set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
	}

	template <uint8_t opcode, Registers src>
	void bit_b3_r8() {
		uint8_t val;
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(3);
		} else {
			val = get_register(src);
			set_cycle(2);
		}
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		set_flag(FlagRegisters::h, 1);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::z, (val & (1 << bit_loc)) == 0);
	}

	template <uint8_t opcode, Registers src>
	void res_b3_r8() {
		uint8_t val;
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(4);
		} else {
			val = get_register(src);
			set_cycle(2);
		}
		uint8_t bit_loc = ((opcode - 0x80) >> 3);
		val &= ~(1 << bit_loc);
		if (src == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), val);
		} else {
			set_register(src, val);
		}
	}

	template <uint8_t opcode, Registers src>
	void set_b3_r8() {
		uint8_t val;
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(4);
		} else {
			val = get_register(src);
			set_cycle(2);
		}
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		val |= (1 << bit_loc);
		if (src == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), val);
		} else {
			set_register(src, val);
		}
	}

	void call_imm16();

	template <Condition condition>
	void call_cond_imm16() {
		pc += 2;
		bool cond = false;
		switch (condition) {
		case Condition::NotZeroFlag:
			if (get_flag(FlagRegisters::z)) {
				cond = true;
			}
			break;
		case Condition::ZeroFlag:
			if (!get_flag(FlagRegisters::z)) {
				cond = true;
			}
			break;
		case Condition::NotCarryFlag:
			if (get_flag(FlagRegisters::c)) {
				cond = true;
			}
			break;
		case Condition::CarryFlag:
			if (!get_flag(FlagRegisters::c)) {
				cond = true;
			}
			break;
		default:
			break;
		}
		if (cond) {
			set_cycle(3);
			return;
		}
		sp -= 2;
		mmap.write_u16(sp, pc);
		pc = mmap.read_u16(pc - 2);
		set_cycle(6);
	}

	template <Registers rec>
	void dec_r16() {
		set_16bitregister(rec, get_16bitregister(rec) - 1);
		set_cycle(2);
	}

	template <Registers rec>
	void dec_r8() {
		uint8_t val;
		uint16_t res;
		if (rec == Registers::HL) {
			uint16_t addr = get_16bitregister(Registers::HL);
			val = mmap.read_u8(addr);
			res = val - 1;
			mmap.write_u8(addr, static_cast<uint8_t>(res));
			set_cycle(3);
		} else {
			val = get_register(rec);
			res = val - 1;
			set_register(rec, static_cast<uint8_t>(res));
			set_cycle(1);
		}
		set_flag(FlagRegisters::z, static_cast<uint8_t>(res) == 0);
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::h, (val & 0xf) < (1 & 0xf));
	}

	template <Registers rec>
	void inc_r16() {
		set_16bitregister(rec, get_16bitregister(rec) + 1);
		set_cycle(2);
	}

	template <Registers rec>
	void inc_r8() {
		uint8_t val;
		uint16_t res;
		if (rec == Registers::HL) {
			uint16_t addr = get_16bitregister(Registers::HL);
			val = mmap.read_u8(addr);
			res = val + 1;
			mmap.write_u8(addr, static_cast<uint8_t>(res));
			set_cycle(3);
		} else {
			val = get_register(rec);
			res = val + 1;
			set_register(rec, static_cast<uint8_t>(res));
			set_cycle(1);
		}
		set_flag(FlagRegisters::z, static_cast<uint8_t>(res) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, ((val & 0xF) + (1 & 0xF)) > 0xF);
	}

	void jr_imm8();
	void jp_imm16();
	void jp_hl();

	template <Condition condition>
	void jr_cond_imm8() {
		bool offset = false;
		uint8_t val = mmap.read_u8(pc);
		pc += 1;
		switch (condition) {
		case Condition::NotZeroFlag:
			offset = get_flag(FlagRegisters::z) == 0;
			break;
		case Condition::ZeroFlag:
			offset = get_flag(FlagRegisters::z) == 1;
			break;
		case Condition::NotCarryFlag:
			offset = get_flag(FlagRegisters::c) == 0;
			break;
		case Condition::CarryFlag:
			offset = get_flag(FlagRegisters::c) == 1;
			break;
		default:
			break;
		}
		if (offset) {
			if (val > 127) {
				pc -= (uint16_t)(255 - val + 1);
			} else {
				pc += (uint16_t)val;
			}
			set_cycle(3);
		} else {
			set_cycle(2);
		}
	}

	template <Condition condition>
	void jp_cond_imm16() {
		bool offset = false;
		uint16_t val;
		pc += 2;
		val = mmap.read_u16(pc - 2);
		switch (condition) {
		case Condition::NotZeroFlag:
			offset = get_flag(FlagRegisters::z) == 0;
			break;
		case Condition::ZeroFlag:
			offset = get_flag(FlagRegisters::z) == 1;
			break;
		case Condition::NotCarryFlag:
			offset = get_flag(FlagRegisters::c) == 0;
			break;
		case Condition::CarryFlag:
			offset = get_flag(FlagRegisters::c) == 1;
			break;
		default:
			break;
		}
		if (offset) {
			pc = val;
			set_cycle(4);
		} else {
			set_cycle(3);
		}
	}

	template <Registers rec, Registers src>
	void ld_r8_r8() {
		uint8_t val;
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
		} else {
			val = get_register(src);
		}
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), val);
		} else {
			set_register(rec, val);
		}
		set_cycle(1);
		if (src == Registers::HL || rec == Registers::HL) {
			set_cycle(1);
		}
	}

	template <Registers rec>
	void ld_r16_a() {
		uint16_t address = get_16bitregister(rec);
		mmap.write_u8(address, get_register(Registers::A));
		set_cycle(2);
	}

	template <Registers src>
	void ld_a_r16() {
		uint16_t address = get_16bitregister(src);
		set_register(Registers::A, mmap.read_u8(address));
		set_cycle(2);
	}

	template <Registers rec>
	void ld_r8_imm8() {
		uint8_t val = mmap.read_u8(pc);
		pc += 1;
		set_register(rec, val);
		set_cycle(2);
	}

	template <Registers rec>
	void ld_r16_imm16() {
		uint16_t val = mmap.read_u16(pc);
		pc += 2;
		set_16bitregister(rec, val);
		set_cycle(3);
	}

	template <Registers rec>
	void pop_r16stk() {
		uint16_t val = mmap.read_u16(sp);
		if constexpr (rec == Registers::AF) {
			val &= 0xFFF0;
		}
		set_16bitregister(rec, val);
		sp += 2;
		set_cycle(3);
	}

	template <Registers src>
	void push_r16stk() {
		uint16_t val = get_16bitregister(src);
		sp -= 2;
		mmap.write_u16(sp, val);
		set_cycle(4);
	}

	void ld_hl_imm8();
	void ld_a_imm16();
	void ld_imm16_a();
	void ld_imm16_sp();
	void ld_hl_sp_imm8();
	void ldh_a_imm8();
	void ldh_imm8_a();
	void ld_c_a();
	void ld_a_c();
	void ld_sp_hl();
	void ld_i_hl_a();
	void ld_i_a_hl();
	void ld_d_hl_a();
	void ld_d_a_hl();

	template <Registers rec>
	void swap_r8() {
		uint8_t val;
		if (rec == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			mmap.write_u8(get_16bitregister(Registers::HL), (val & 0xF) << 4);
			mmap.write_u8(get_16bitregister(Registers::HL),
			              mmap.read_u8(get_16bitregister(Registers::HL)) | ((val & 0xF0) >> 4));
			set_cycle(4);
		} else {
			val = get_register(rec);
			set_register(rec, (val & 0xF) << 4);
			set_register(rec, get_register(rec) | ((val & 0xF0) >> 4));
			set_cycle(2);
		}
		set_flag(FlagRegisters::z, val == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::c, 0);
		set_flag(FlagRegisters::h, 0);
	}

	void nop();
	void daa();
	void cpl();
	void scf();
	void ccf();
	void stop();
	void halt();
	void di();
	void ei();

	template <uint16_t value>
	void rst_tg3() {
		sp -= 2;
		mmap.write_u16(sp, pc);

		pc = value;
		set_cycle(4);
	}

	void ret();
	void reti();

	template <Condition condition>
	void ret_cond() {
		bool cond = false;
		switch (condition) {
		case Condition::NotZeroFlag:
			if (get_flag(FlagRegisters::z)) {
				cond = true;
			}
			break;
		case Condition::ZeroFlag:
			if (!get_flag(FlagRegisters::z)) {
				cond = true;
			}
			break;
		case Condition::NotCarryFlag:
			if (get_flag(FlagRegisters::c)) {
				cond = true;
			}
			break;
		case Condition::CarryFlag:
			if (!get_flag(FlagRegisters::c)) {
				cond = true;
			}
			break;
		default:
			break;
		}
		if (cond) {
			set_cycle(2);
			return;
		}
		set_cycle(1);
		ret();
	}

	uint8_t get_rlc(uint8_t val, bool reset = false);
	uint8_t get_rrc(uint8_t val, bool reset = false);
	uint8_t get_rr(uint8_t val, bool reset = false);
	uint8_t get_rl(uint8_t val, bool reset = false);
	void rlca();
	void rla();
	void rrca();
	void rra();

	template <Registers rec>
	void rlc_r8() {
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), get_rlc(mmap.read_u8(get_16bitregister(Registers::HL))));
			set_cycle(4);
		} else {
			set_register(rec, get_rlc(get_register(rec)));
			set_cycle(2);
		}
	}

	template <Registers rec>
	void rl_r8() {
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), get_rl(mmap.read_u8(get_16bitregister(Registers::HL))));
			set_cycle(4);
		} else {
			set_register(rec, get_rl(get_register(rec)));
			set_cycle(2);
		}
	}

	template <Registers rec>
	void rrc_r8() {
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), get_rrc(mmap.read_u8(get_16bitregister(Registers::HL))));
			set_cycle(4);
		} else {
			set_register(rec, get_rrc(get_register(rec)));
			set_cycle(2);
		}
	}

	template <Registers rec>
	void rr_r8() {
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), get_rr(mmap.read_u8(get_16bitregister(Registers::HL))));
			set_cycle(4);
		} else {
			set_register(rec, get_rr(get_register(rec)));
			set_cycle(2);
		}
	}

	template <Registers rec>
	void sla_r8() {
		uint8_t val;
		if (rec == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(4);
		} else {
			val = get_register(rec);
			set_cycle(2);
		}
		set_flag(FlagRegisters::c, (val >> 7) & 0x01);
		val <<= 1;
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 0);
		set_flag(FlagRegisters::z, val == 0);
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), val);
		} else {
			set_register(rec, val);
		}
	}

	template <Registers rec>
	void sra_r8() {
		uint8_t val;
		if (rec == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(4);
		} else {
			val = get_register(rec);
			set_cycle(2);
		}
		set_flag(FlagRegisters::c, (val >> 0) & 1);
		val = (val >> 1) | (val & 0x80);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 0);
		set_flag(FlagRegisters::z, val == 0);
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), val);
		} else {
			set_register(rec, val);
		}
	}

	template <Registers rec>
	void srl_r8() {
		uint8_t val;
		if (rec == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(4);
		} else {
			val = get_register(rec);
			set_cycle(2);
		}
		set_flag(FlagRegisters::c, val & 0x01);
		val >>= 1;
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 0);
		set_flag(FlagRegisters::z, val == 0);
		if (rec == Registers::HL) {
			mmap.write_u8(get_16bitregister(Registers::HL), val);
		} else {
			set_register(rec, val);
		}
	}

	void sub_a_imm8();
	void sbc_a_imm8();

	template <Registers src>
	void sub_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		uint16_t sub = a_val - val;
		set_register(Registers::A, static_cast<uint8_t>(sub));
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sub) == 0);
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
		set_flag(FlagRegisters::c, val > a_val);
	}

	template <Registers src>
	void sbc_a_r8() {
		uint8_t val;
		uint8_t a_val = get_register(Registers::A);
		if (src == Registers::HL) {
			val = mmap.read_u8(get_16bitregister(Registers::HL));
			set_cycle(2);
		} else {
			val = get_register(src);
			set_cycle(1);
		}
		uint8_t carry = get_flag(FlagRegisters::c);
		uint16_t sbc = a_val - val - carry;
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::c, (sbc >> 8) != 0);
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sbc) == 0);
		set_flag(FlagRegisters::h, (a_val & 0xF) < (val & 0xF) + carry);
		set_register(Registers::A, static_cast<uint8_t>(sbc));
	}

	void execute_instruction();

	void debug_print(bool prefix);

	inline void set_cycle(uint8_t c) {
		m_cycle += c;
		t_cycle += c * 4;
	}

	void handle_interrupt();
	void process_interrupt(InterruptType i);

public:
	uint64_t debug_count;
	Cpu(Decoder dec, const std::string path);
	~Cpu();

	uint8_t get_register(Registers reg) const;
	uint16_t get_16bitregister(Registers reg) const;
	uint8_t get_flag(uint8_t flag) const;
	void set_16bitregister(Registers reg, uint16_t val);
	void set_register(Registers reg, uint8_t val);
	void set_flag(uint8_t flag, uint8_t val);

	inline PixelProcessingUnit &get_ppu() {
		return ppu;
	}
	inline MemoryMap &get_mmap() {
		return mmap;
	}

	void tick();
	void event_handler();
	void handle_input(SDL_Event &e);
	inline bool status() {
		return ppu.status();
	}
	inline void set_status(bool val) {
		ppu.set_status(val);
	}
	inline void close() {
		ppu.close();
	}
	inline void set_interrupt(InterruptType i) {
		interrupt |= static_cast<uint8_t>(i);
	}
	inline void overwrite_interrupt(uint8_t val) {
		interrupt = val;
	}
	inline uint8_t get_interrupt() const {
		return interrupt;
	}
	inline void set_interrupt_enable(uint8_t val) {
		interrupt_enable_register = val;
	}
	inline uint8_t get_interrupt_enable() {
		return interrupt_enable_register;
	}
	inline uint8_t get_timer_divider() {
		return timer_divider;
	}
	inline void reset_timer_divider() {
		timer_divider = 0;
	}

	inline uint8_t get_timer_counter() {
		return timer_counter;
	}
	inline void set_timer_counter(uint8_t val) {
		timer_counter = val;
	}

	inline uint8_t get_timer_module() {
		return timer_modulo;
	}
	inline void set_timer_modulo(uint8_t val) {
		timer_modulo = val;
	}

	inline uint8_t get_timer_control() {
		return (timer_enable << 2 | timer_clock_select << 0);
	}
	inline void set_timer_control(uint8_t val) {
		timer_enable = (val >> 2) & 1;
		if (!timer_enable) {
			t_cycle = 0;
		}
		timer_clock_select = (val >> 0) & 3;
	}

	void serialize(const std::string &file);
	void deserialize(const std::string &file);
};

#endif

// 09:01
// 11:01