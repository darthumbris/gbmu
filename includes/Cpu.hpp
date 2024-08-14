#ifndef CPU_HPP
#define CPU_HPP

#include "Decoder.hpp"
#include "Interruptor.hpp"
#include "MemoryMap.hpp"
#include "Operand.hpp"
#include "PixelProcessingUnit.hpp"
#include "debug.hpp"
#include <array>
#include <bitset>
#include <cstdint>
#include <cstdio>
#include <math.h>

using namespace Dict;

enum FlagRegisters { c = 4, h = 5, n = 6, z = 7 };

enum Condition {
	NotZeroFlag,
	ZeroFlag,
	NotCarryFlag,
	CarryFlag,
};

enum InstructionState {
	StateReady,
	StateReadingWord,
	StateReadingByte,
};

enum InstructionList {
	Unprefixed,
	Prefixed
};

// TODO handle Audio (is needed for proper interrupt times etc)

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
	Interruptor interruptor;
	bool halted = false;
	bool locked = false;
	uint16_t m_cycle;
	uint16_t t_cycle;
	int16_t halt_cycle = 0;
	uint8_t opcode = 0;
	uint8_t accurate_opcode_state = 0;
	uint8_t read_cache = 0;
	bool branched = false;
	InstructionList instruction = InstructionList::Unprefixed;

	bool cgb_speed = false;
	uint16_t speed_multiplier = 0;

	std::string rom_path;

	using OpsFn = void (Cpu::*)();

	OpsFn instructions[2][256];
	void set_instructions();

	void prefix();

	template <uint8_t op>
	void unimplemented() {
		DEBUG_MSG("Unimplemented opcode: %#04x\n", op);
	}

	//$D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD
	template <uint8_t op>
	void lockup() {
		locked = true;
		DEBUG_MSG("Illegal instruction. Hard-Locks the Cpu. opcode: %#04x\n", op);
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
		uint8_t a_val = get_register(Registers::A);
		uint8_t val = get_register(src);
		uint16_t sum = a_val + val;
		set_register(Registers::A, static_cast<uint8_t>(sum));
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
		set_flag(FlagRegisters::c, (sum >> 8) != 0);
	}

	void add_a_r8_hl();

	template <Registers src>
	void add_hl_r16() {
		uint16_t val = get_16bitregister(src);
		uint16_t hl_val = get_16bitregister(Registers::HL);
		set_16bitregister(Registers::HL, static_cast<uint16_t>(val + hl_val));
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, (val & 0xfff) + (hl_val & 0xfff) > 0xfff);
		set_flag(FlagRegisters::c, (val + hl_val) >> 16);
	}

	template <Registers src>
	void adc_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(Registers::A);
		uint8_t carry = get_flag(FlagRegisters::c);
		uint16_t sum = a_val + val + carry;
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sum) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
		set_flag(FlagRegisters::c, (sum >> 8) != 0);
		set_register(Registers::A, sum);
	}

	void adc_a_r8_hl();

	void xor_a_imm8();
	void or_a_imm8();
	void cp_a_imm8();
	void and_a_imm8();

	template <Registers src>
	void and_a_r8() {
		uint8_t a_val = get_register(Registers::A);
		uint8_t val = get_register(src);
		set_register(Registers::A, a_val & val);
		set_flag(FlagRegisters::z, (a_val & val) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 1);
		set_flag(FlagRegisters::c, 0);
	}

	void and_a_r8_hl();

	template <Registers src>
	void xor_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(Registers::A);
		a_val ^= val;
		set_register(Registers::A, a_val);
		set_register(Registers::F, 0);
		set_flag(FlagRegisters::z, a_val == 0);
	}

	void xor_a_r8_hl();

	template <Registers src>
	void or_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(Registers::A);
		set_register(Registers::A, a_val | val);
		set_register(Registers::F, 0);
		set_flag(FlagRegisters::z, (a_val | val) == 0);
	}

	void or_a_r8_hl();

	template <Registers src>
	void cp_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(Registers::A);
		set_flag(FlagRegisters::z, (a_val == val));
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::c, a_val < val);
		set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
	}

	void cp_a_r8_hl();

	template <uint8_t opcode, Registers src>
	void bit_b3_r8() {
		uint8_t val = get_register(src);
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		set_flag(FlagRegisters::h, 1);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::z, (val & (1 << bit_loc)) == 0);
	}

	template <uint8_t opcode>
	void bit_b3_r8_hl() {
		uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		set_flag(FlagRegisters::h, 1);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::z, (val & (1 << bit_loc)) == 0);
	}

	template <uint8_t opcode, Registers src>
	void res_b3_r8() {
		uint8_t val = get_register(src);
		uint8_t bit_loc = ((opcode - 0x80) >> 3);
		val &= ~(1 << bit_loc);
		set_register(src, val);
	}

	template <uint8_t opcode>
	void res_b3_r8_hl() {
		if (accurate_opcode_state == StateReadingWord) {
			read_cache = mmap.read_u8(get_16bitregister(Registers::HL));
			return;
		}
		uint8_t bit_loc = ((opcode - 0x80) >> 3);
		if (opcode == 0x86) {
			DEBUG_MSG("address writing to: %#06X val: %u\n", get_16bitregister(Registers::HL), read_cache);
		}
		read_cache &= ~(1 << bit_loc);
		mmap.write_u8(get_16bitregister(Registers::HL), read_cache);
	}

	template <uint8_t opcode, Registers src>
	void set_b3_r8() {
		uint8_t val = get_register(src);
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		val |= (1 << bit_loc);
		set_register(src, val);
	}

	template <uint8_t opcode>
	void set_b3_r8_hl() {
		if (accurate_opcode_state == StateReadingWord) {
			read_cache = mmap.read_u8(get_16bitregister(Registers::HL));
			return;
		}
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		read_cache |= (1 << bit_loc);
		mmap.write_u8(get_16bitregister(Registers::HL), read_cache);
	}

	void call_imm16();

	void call_cond_nz();
	void call_cond_z();
	void call_cond_nc();
	void call_cond_c();
	void call_cond_imm16(Condition condition);

	template <Registers rec>
	void dec_r16() {
		set_16bitregister(rec, get_16bitregister(rec) - 1);
	}

	template <Registers rec>
	void dec_r8() {
		uint8_t val;
		uint16_t res;
		val = get_register(rec);
		res = val - 1;
		set_register(rec, static_cast<uint8_t>(res));
		set_flag(FlagRegisters::z, static_cast<uint8_t>(res) == 0);
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::h, (val & 0xf) < (1 & 0xf));
	}

	void dec_r8_hl();

	template <Registers rec>
	void inc_r16() {
		set_16bitregister(rec, get_16bitregister(rec) + 1);
	}

	template <Registers rec>
	void inc_r8() {
		uint8_t val;
		uint16_t res;
		val = get_register(rec);
		res = val + 1;
		set_register(rec, static_cast<uint8_t>(res));
		set_flag(FlagRegisters::z, static_cast<uint8_t>(res) == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, ((val & 0xF) + (1 & 0xF)) > 0xF);
	}

	void inc_r8_hl();

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
			branched = true;
			if (val > 127) {
				pc -= (uint16_t)(255 - val + 1);
			} else {
				pc += (uint16_t)val;
			}
			if (condition == Condition::ZeroFlag) {
				DEBUG_MSG("set pc to: %u\n", pc);
			}
		}
	}

	void jp_cond_nz();
	void jp_cond_z();
	void jp_cond_nc();
	void jp_cond_c();
	void jp_cond_imm16(Condition condition);

	template <Registers rec, Registers src>
	void ld_r8_r8() {
		uint8_t val = get_register(src);
		set_register(rec, val);
	}

	template <Registers rec>
	void ld_r8_hl() {
		uint8_t val = mmap.read_u8(get_16bitregister(Registers::HL));
		set_register(rec, val);
	}

	template <Registers src>
	void ld_hl_r8() {
		uint8_t val = get_register(src);
		mmap.write_u8(get_16bitregister(Registers::HL), val);
	}

	template <Registers rec>
	void ld_r16_a() {
		uint16_t address = get_16bitregister(rec);
		mmap.write_u8(address, get_register(Registers::A));
	}

	template <Registers src>
	void ld_a_r16() {
		uint16_t address = get_16bitregister(src);
		set_register(Registers::A, mmap.read_u8(address));
	}

	template <Registers rec>
	void ld_r8_imm8() {
		uint8_t val = mmap.read_u8(pc);
		pc += 1;
		set_register(rec, val);
	}

	template <Registers rec>
	void ld_r16_imm16() {
		uint16_t val = mmap.read_u16(pc);
		pc += 2;
		set_16bitregister(rec, val);
	}

	template <Registers rec>
	void pop_r16stk() {
		uint16_t val = mmap.read_u16(sp);
		if constexpr (rec == Registers::AF) {
			val &= 0xFFF0;
		}
		set_16bitregister(rec, val);
		sp += 2;
	}

	template <Registers src>
	void push_r16stk() {
		uint16_t val = get_16bitregister(src);
		sp -= 2;
		mmap.write_u16(sp, val);
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
		val = get_register(rec);
		set_register(rec, (val & 0xF) << 4);
		set_register(rec, get_register(rec) | ((val & 0xF0) >> 4));
		set_flag(FlagRegisters::z, val == 0);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::c, 0);
		set_flag(FlagRegisters::h, 0);
	}

	void swap_r8_hl();

	void nop();
	void daa();
	void cpl();
	void scf();
	void ccf();
	void stop();
	void halt();
	void di();
	void ei();

	void rst_00();
	void rst_08();
	void rst_10();
	void rst_18();
	void rst_20();
	void rst_28();
	void rst_30();
	void rst_38();
	void rst_tg3(uint16_t value);

	void ret();
	void reti();

	void ret_nz();
	void ret_z();
	void ret_nc();
	void ret_c();

	void ret_cond(Condition condition);

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
		set_register(rec, get_rlc(get_register(rec)));
	}

	void rlc_r8_hl();

	template <Registers rec>
	void rl_r8() {
		set_register(rec, get_rl(get_register(rec)));
	}

	void rl_r8_hl();

	template <Registers rec>
	void rrc_r8() {
		set_register(rec, get_rrc(get_register(rec)));
	}

	void rrc_r8_hl();

	template <Registers rec>
	void rr_r8() {
		set_register(rec, get_rr(get_register(rec)));
	}

	void rr_r8_hl();

	template <Registers rec>
	void sla_r8() {
		uint8_t val = get_register(rec);
		set_flag(FlagRegisters::c, (val >> 7) & 0x01);
		val <<= 1;
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 0);
		set_flag(FlagRegisters::z, val == 0);
		set_register(rec, val);
	}

	void sla_r8_hl();

	template <Registers rec>
	void sra_r8() {
		uint8_t val = get_register(rec);
		set_flag(FlagRegisters::c, (val >> 0) & 1);
		val = (val >> 1) | (val & 0x80);
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 0);
		set_flag(FlagRegisters::z, val == 0);
		set_register(rec, val);
	}

	void sra_r8_hl();

	template <Registers rec>
	void srl_r8() {
		uint8_t val;
		val = get_register(rec);
		set_flag(FlagRegisters::c, val & 0x01);
		val >>= 1;
		set_flag(FlagRegisters::n, 0);
		set_flag(FlagRegisters::h, 0);
		set_flag(FlagRegisters::z, val == 0);
		set_register(rec, val);
	}

	void srl_r8_hl();

	void sub_a_imm8();
	void sbc_a_imm8();

	template <Registers src>
	void sub_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(Registers::A);
		uint16_t sub = a_val - val;
		set_register(Registers::A, static_cast<uint8_t>(sub));
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sub) == 0);
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::h, (a_val & 0xf) < (val & 0xf));
		set_flag(FlagRegisters::c, val > a_val);
	}

	void sub_a_r8_hl();

	template <Registers src>
	void sbc_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(Registers::A);
		uint8_t carry = get_flag(FlagRegisters::c);
		uint16_t sbc = a_val - val - carry;
		set_flag(FlagRegisters::n, 1);
		set_flag(FlagRegisters::c, (sbc >> 8) != 0);
		set_flag(FlagRegisters::z, static_cast<uint8_t>(sbc) == 0);
		set_flag(FlagRegisters::h, (a_val & 0xF) < (val & 0xF) + carry);
		set_register(Registers::A, static_cast<uint8_t>(sbc));
	}

	void sbc_a_r8_hl();

	void handle_instruction();
	void fetch_instruction();
	void set_cycles_left();
	void execute_instruction();
	void decrement_pc();

	void debug_print(bool prefix);

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

	uint16_t cycle_speed(uint16_t cycle);

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

	void process_interrupt(InterruptType i);

	inline Interruptor &interrupt() {
		return interruptor;
	}

	inline void set_cycle(uint8_t c) {
		m_cycle += c;
		if (speed_multiplier) {
			t_cycle += (c * 4) >> 1;
		} else {
			t_cycle += c * 4;
		}
	}

	inline void set_cycle_16(uint16_t c) {
		m_cycle += c / 4;
		t_cycle += c;
	}

	inline bool is_halted() const {
		return halted;
	}

	void halt_cpu() {
		halted = true;
	}

	void unhalt_cpu() {
		halted = false;
	}

	inline bool get_cgb_speed() const {
		return cgb_speed;
	}

	void handle_halt();

	void serialize(const std::string &file);
	void deserialize(const std::string &file);
};

// TODO change the naming
// TODO try using #include <bitset>
inline bool IsSetBit(const uint8_t value, const uint8_t bit) {
	return (value & (0x01 << bit)) != 0;
}

#endif