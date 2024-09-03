#ifndef CPU_HPP
#define CPU_HPP

#include "AudioProcessingUnit/AudioProcessingUnit.hpp"
#include "Decoder.hpp"
#include "Interruptor.hpp"
#include "MemoryMap.hpp"
#include "Operand.hpp"
#include "PixelProcessingUnit.hpp"
#include "debug.hpp"
#include <SDL2/SDL_stdinc.h>
#include <array>
#include <cstdint>
#include <cstdio>
#include <math.h>

using namespace Dict;

enum flag_registers { c = 4, h = 5, n = 6, z = 7 };

enum condition {
	NotZeroFlag,
	ZeroFlag,
	NotCarryFlag,
	CarryFlag,
};

enum class instruction_state {
	Ready,
	ReadingWord,
	ReadingByte,
};

struct options {
	bool matrix;
	bool force_dmg;
	bool force_cgb;
	uint8_t darkening;
	bool color_correction;
	std::string path;
};

enum instruction_list { Unprefixed, Prefixed };

class Cpu {
private:
	/*
	 * registers (
	    AF Accumalator & Flag register,
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
	AudioProcessingUnit apu;
	Interruptor interruptor;
	bool halted = false;
	bool locked = false;
	bool paused = false;
	uint16_t m_cycle;
	uint16_t t_cycle;
	int16_t halt_cycle = 0;
	uint8_t opcode = 0;
	instruction_state accurate_opcode_state = instruction_state::Ready;
	uint8_t read_cache = 0;
	bool branched = false;
	instruction_list instruction = instruction_list::Unprefixed;

	bool cgb_speed = false;
	uint16_t speed_multiplier = 0;

	std::string rom_path;
	options load_options;

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

	void add_a_imm8();
	void add_sp_imm8();
	void adc_a_imm8();

	template <registers src>
	void add_a_r8() {
		uint8_t a_val = get_register(registers::A);
		uint8_t val = get_register(src);
		uint16_t sum = a_val + val;
		set_register(registers::A, static_cast<uint8_t>(sum));
		set_flag(flag_registers::z, static_cast<uint8_t>(sum) == 0);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, ((a_val & 0xF) + (val & 0xF)) > 0xF);
		set_flag(flag_registers::c, (sum >> 8) != 0);
	}

	void add_a_r8_hl();

	template <registers src>
	void add_hl_r16() {
		uint16_t val = get_16bitregister(src);
		uint16_t hl_val = get_16bitregister(registers::HL);
		set_16bitregister(registers::HL, static_cast<uint16_t>(val + hl_val));
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, (val & 0xfff) + (hl_val & 0xfff) > 0xfff);
		set_flag(flag_registers::c, (val + hl_val) >> 16);
	}

	template <registers src>
	void adc_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(registers::A);
		uint8_t carry = get_flag(flag_registers::c);
		uint16_t sum = a_val + val + carry;
		set_flag(flag_registers::z, static_cast<uint8_t>(sum) == 0);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, ((a_val & 0xF) + (val & 0xF)) + carry > 0xF);
		set_flag(flag_registers::c, (sum >> 8) != 0);
		set_register(registers::A, sum);
	}

	void adc_a_r8_hl();

	void xor_a_imm8();
	void or_a_imm8();
	void cp_a_imm8();
	void and_a_imm8();

	template <registers src>
	void and_a_r8() {
		uint8_t a_val = get_register(registers::A);
		uint8_t val = get_register(src);
		set_register(registers::A, a_val & val);
		set_flag(flag_registers::z, (a_val & val) == 0);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, 1);
		set_flag(flag_registers::c, 0);
	}

	void and_a_r8_hl();

	template <registers src>
	void xor_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(registers::A);
		a_val ^= val;
		set_register(registers::A, a_val);
		set_register(registers::F, 0);
		set_flag(flag_registers::z, a_val == 0);
	}

	void xor_a_r8_hl();

	template <registers src>
	void or_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(registers::A);
		set_register(registers::A, a_val | val);
		set_register(registers::F, 0);
		set_flag(flag_registers::z, (a_val | val) == 0);
	}

	void or_a_r8_hl();

	template <registers src>
	void cp_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(registers::A);
		set_flag(flag_registers::z, (a_val == val));
		set_flag(flag_registers::n, 1);
		set_flag(flag_registers::c, a_val < val);
		set_flag(flag_registers::h, (a_val & 0xf) < (val & 0xf));
	}

	void cp_a_r8_hl();

	template <uint8_t opcode, registers src>
	void bit_b3_r8() {
		uint8_t val = get_register(src);
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		set_flag(flag_registers::h, 1);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::z, (val & (1 << bit_loc)) == 0);
	}

	template <uint8_t opcode>
	void bit_b3_r8_hl() {
		uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		set_flag(flag_registers::h, 1);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::z, (val & (1 << bit_loc)) == 0);
	}

	template <uint8_t opcode, registers src>
	void res_b3_r8() {
		uint8_t val = get_register(src);
		uint8_t bit_loc = ((opcode - 0x80) >> 3);
		val &= ~(1 << bit_loc);
		set_register(src, val);
	}

	template <uint8_t opcode>
	void res_b3_r8_hl() {
		if (accurate_opcode_state == instruction_state::ReadingWord) {
			read_cache = mmap.read_u8(get_16bitregister(registers::HL));
			return;
		}
		uint8_t bit_loc = ((opcode - 0x80) >> 3);
		if (opcode == 0x86) {
			DEBUG_MSG("address writing to: %#06X val: %u\n", get_16bitregister(registers::HL), read_cache);
		}
		read_cache &= ~(1 << bit_loc);
		mmap.write_u8(get_16bitregister(registers::HL), read_cache);
	}

	template <uint8_t opcode, registers src>
	void set_b3_r8() {
		uint8_t val = get_register(src);
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		val |= (1 << bit_loc);
		set_register(src, val);
	}

	template <uint8_t opcode>
	void set_b3_r8_hl() {
		if (accurate_opcode_state == instruction_state::ReadingWord) {
			read_cache = mmap.read_u8(get_16bitregister(registers::HL));
			return;
		}
		uint8_t bit_loc = (opcode >> 3) & 0x7;
		read_cache |= (1 << bit_loc);
		mmap.write_u8(get_16bitregister(registers::HL), read_cache);
	}

	void call_imm16();

	void call_cond_nz();
	void call_cond_z();
	void call_cond_nc();
	void call_cond_c();
	void call_cond_imm16(condition condition);

	template <registers rec>
	void dec_r16() {
		set_16bitregister(rec, get_16bitregister(rec) - 1);
	}

	template <registers rec>
	void dec_r8() {
		uint8_t val;
		uint16_t res;
		val = get_register(rec);
		res = val - 1;
		set_register(rec, static_cast<uint8_t>(res));
		set_flag(flag_registers::z, static_cast<uint8_t>(res) == 0);
		set_flag(flag_registers::n, 1);
		set_flag(flag_registers::h, (val & 0xf) < (1 & 0xf));
	}

	void dec_r8_hl();

	template <registers rec>
	void inc_r16() {
		set_16bitregister(rec, get_16bitregister(rec) + 1);
	}

	template <registers rec>
	void inc_r8() {
		uint8_t val;
		uint16_t res;
		val = get_register(rec);
		res = val + 1;
		set_register(rec, static_cast<uint8_t>(res));
		set_flag(flag_registers::z, static_cast<uint8_t>(res) == 0);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, ((val & 0xF) + (1 & 0xF)) > 0xF);
	}

	void inc_r8_hl();

	void jr_imm8();
	void jp_imm16();
	void jp_hl();

	template <condition condition>
	void jr_cond_imm8() {
		bool offset = false;
		uint8_t val = mmap.read_u8(pc);
		pc += 1;
		switch (condition) {
		case condition::NotZeroFlag:
			offset = get_flag(flag_registers::z) == 0;
			break;
		case condition::ZeroFlag:
			offset = get_flag(flag_registers::z) == 1;
			break;
		case condition::NotCarryFlag:
			offset = get_flag(flag_registers::c) == 0;
			break;
		case condition::CarryFlag:
			offset = get_flag(flag_registers::c) == 1;
			break;
		default:
			break;
		}
		if (offset) {
			branched = true;
			if (val > 127) {
				pc -= static_cast<uint16_t>(255 - val + 1);
			} else {
				pc += static_cast<uint16_t>(val);
			}
			if (condition == condition::ZeroFlag) {
				DEBUG_MSG("set pc to: %u\n", pc);
			}
		}
	}

	void jp_cond_nz();
	void jp_cond_z();
	void jp_cond_nc();
	void jp_cond_c();
	void jp_cond_imm16(condition condition);

	template <registers rec, registers src>
	void ld_r8_r8() {
		uint8_t val = get_register(src);
		set_register(rec, val);
	}

	template <registers rec>
	void ld_r8_hl() {
		uint8_t val = mmap.read_u8(get_16bitregister(registers::HL));
		set_register(rec, val);
	}

	template <registers src>
	void ld_hl_r8() {
		uint8_t val = get_register(src);
		mmap.write_u8(get_16bitregister(registers::HL), val);
	}

	template <registers rec>
	void ld_r16_a() {
		uint16_t address = get_16bitregister(rec);
		mmap.write_u8(address, get_register(registers::A));
	}

	template <registers src>
	void ld_a_r16() {
		uint16_t address = get_16bitregister(src);
		set_register(registers::A, mmap.read_u8(address));
	}

	template <registers rec>
	void ld_r8_imm8() {
		uint8_t val = mmap.read_u8(pc);
		pc += 1;
		set_register(rec, val);
	}

	template <registers rec>
	void ld_r16_imm16() {
		uint16_t val = mmap.read_u16(pc);
		pc += 2;
		set_16bitregister(rec, val);
	}

	template <registers rec>
	void pop_r16stk() {
		uint16_t val = mmap.read_u16(sp);
		if constexpr (rec == registers::AF) {
			val &= 0xFFF0;
		}
		set_16bitregister(rec, val);
		sp += 2;
	}

	template <registers src>
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

	template <registers rec>
	void swap_r8() {
		uint8_t val;
		val = get_register(rec);
		set_register(rec, (val & 0xF) << 4);
		set_register(rec, get_register(rec) | ((val & 0xF0) >> 4));
		set_flag(flag_registers::z, val == 0);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::c, 0);
		set_flag(flag_registers::h, 0);
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

	void ret_cond(condition condition);

	uint8_t get_rlc(uint8_t val, bool reset = false);
	uint8_t get_rrc(uint8_t val, bool reset = false);
	uint8_t get_rr(uint8_t val, bool reset = false);
	uint8_t get_rl(uint8_t val, bool reset = false);
	void rlca();
	void rla();
	void rrca();
	void rra();

	template <registers rec>
	void rlc_r8() {
		set_register(rec, get_rlc(get_register(rec)));
	}

	void rlc_r8_hl();

	template <registers rec>
	void rl_r8() {
		set_register(rec, get_rl(get_register(rec)));
	}

	void rl_r8_hl();

	template <registers rec>
	void rrc_r8() {
		set_register(rec, get_rrc(get_register(rec)));
	}

	void rrc_r8_hl();

	template <registers rec>
	void rr_r8() {
		set_register(rec, get_rr(get_register(rec)));
	}

	void rr_r8_hl();

	template <registers rec>
	void sla_r8() {
		uint8_t val = get_register(rec);
		set_flag(flag_registers::c, (val >> 7) & 0x01);
		val <<= 1;
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, 0);
		set_flag(flag_registers::z, val == 0);
		set_register(rec, val);
	}

	void sla_r8_hl();

	template <registers rec>
	void sra_r8() {
		uint8_t val = get_register(rec);
		set_flag(flag_registers::c, (val >> 0) & 1);
		val = (val >> 1) | (val & 0x80);
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, 0);
		set_flag(flag_registers::z, val == 0);
		set_register(rec, val);
	}

	void sra_r8_hl();

	template <registers rec>
	void srl_r8() {
		uint8_t val;
		val = get_register(rec);
		set_flag(flag_registers::c, val & 0x01);
		val >>= 1;
		set_flag(flag_registers::n, 0);
		set_flag(flag_registers::h, 0);
		set_flag(flag_registers::z, val == 0);
		set_register(rec, val);
	}

	void srl_r8_hl();

	void sub_a_imm8();
	void sbc_a_imm8();

	template <registers src>
	void sub_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(registers::A);
		uint16_t sub = a_val - val;
		set_register(registers::A, static_cast<uint8_t>(sub));
		set_flag(flag_registers::z, static_cast<uint8_t>(sub) == 0);
		set_flag(flag_registers::n, 1);
		set_flag(flag_registers::h, (a_val & 0xf) < (val & 0xf));
		set_flag(flag_registers::c, val > a_val);
	}

	void sub_a_r8_hl();

	template <registers src>
	void sbc_a_r8() {
		uint8_t val = get_register(src);
		uint8_t a_val = get_register(registers::A);
		uint8_t carry = get_flag(flag_registers::c);
		uint16_t sbc = a_val - val - carry;
		set_flag(flag_registers::n, 1);
		set_flag(flag_registers::c, (sbc >> 8) != 0);
		set_flag(flag_registers::z, static_cast<uint8_t>(sbc) == 0);
		set_flag(flag_registers::h, (a_val & 0xF) < (val & 0xF) + carry);
		set_register(registers::A, static_cast<uint8_t>(sbc));
	}

	void sbc_a_r8_hl();

	uint8_t handle_instruction();
	void fetch_instruction();
	void set_cycles_left();
	void execute_instruction();
	void decrement_pc();

	void debug_print(bool prefix);

public:
	uint64_t debug_count;
	Cpu(Decoder dec, const options options);
	~Cpu();

	uint8_t get_register(registers reg) const;
	uint16_t get_16bitregister(registers reg) const;
	uint8_t get_flag(uint8_t flag) const;
	void set_16bitregister(registers reg, uint16_t val);
	void set_register(registers reg, uint8_t val);
	void set_flag(uint8_t flag, uint8_t val);

	inline PixelProcessingUnit &get_ppu() {
		return ppu;
	}

	inline MemoryMap &get_mmap() {
		return mmap;
	}

	inline AudioProcessingUnit &get_apu() {
		return apu;
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

	void process_interrupt(interrupt_type i);

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

	void unhalt_cpu() {
		halted = false;
	}

	inline bool get_cgb_speed() const {
		return cgb_speed;
	}

	void handle_halt();

	void reset();

	void serialize(const std::string &file);
	void deserialize(const std::string &file);
};

#endif