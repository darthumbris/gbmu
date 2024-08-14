#include "Cpu.hpp"

//TODO functions with an if statement for HL make seperate function for them

void Cpu::set_instructions() {
#define u instructions[instruction_list::Unprefixed]
	u[0x00] = &Cpu::nop;
	u[0x01] = &Cpu::ld_r16_imm16<Registers::BC>;
	u[0x02] = &Cpu::ld_r16_a<Registers::BC>;
	u[0x03] = &Cpu::inc_r16<Registers::BC>;
	u[0x04] = &Cpu::inc_r8<Registers::B>;
	u[0x05] = &Cpu::dec_r8<Registers::B>;
	u[0x06] = &Cpu::ld_r8_imm8<Registers::B>;
	u[0x07] = &Cpu::rlca;
	u[0x08] = &Cpu::ld_imm16_sp;
	u[0x09] = &Cpu::add_hl_r16<Registers::BC>;
	u[0x0A] = &Cpu::ld_a_r16<Registers::BC>;
	u[0x0B] = &Cpu::dec_r16<Registers::BC>;
	u[0x0C] = &Cpu::inc_r8<Registers::C>;
	u[0x0D] = &Cpu::dec_r8<Registers::C>;
	u[0x0E] = &Cpu::ld_r8_imm8<Registers::C>;
	u[0x0F] = &Cpu::rrca;
	u[0x10] = &Cpu::stop;
	u[0x11] = &Cpu::ld_r16_imm16<Registers::DE>;
	u[0x12] = &Cpu::ld_r16_a<Registers::DE>;
	u[0x13] = &Cpu::inc_r16<Registers::DE>;
	u[0x14] = &Cpu::inc_r8<Registers::D>;
	u[0x15] = &Cpu::dec_r8<Registers::D>;
	u[0x16] = &Cpu::ld_r8_imm8<Registers::D>;
	u[0x17] = &Cpu::rla;
	u[0x18] = &Cpu::jr_imm8;
	u[0x19] = &Cpu::add_hl_r16<Registers::DE>;
	u[0x1A] = &Cpu::ld_a_r16<Registers::DE>;
	u[0x1B] = &Cpu::dec_r16<Registers::DE>;
	u[0x1C] = &Cpu::inc_r8<Registers::E>;
	u[0x1D] = &Cpu::dec_r8<Registers::E>;
	u[0x1E] = &Cpu::ld_r8_imm8<Registers::E>;
	u[0x1F] = &Cpu::rra;
	u[0x20] = &Cpu::jr_cond_imm8<Condition::NotZeroFlag>;
	u[0x21] = &Cpu::ld_r16_imm16<Registers::HL>;
	u[0x22] = &Cpu::ld_i_hl_a;
	u[0x23] = &Cpu::inc_r16<Registers::HL>;
	u[0x24] = &Cpu::inc_r8<Registers::H>;
	u[0x25] = &Cpu::dec_r8<Registers::H>;
	u[0x26] = &Cpu::ld_r8_imm8<Registers::H>;
	u[0x27] = &Cpu::daa;
	u[0x28] = &Cpu::jr_cond_imm8<Condition::ZeroFlag>;
	u[0x29] = &Cpu::add_hl_r16<Registers::HL>;
	u[0x2A] = &Cpu::ld_i_a_hl;
	u[0x2B] = &Cpu::dec_r16<Registers::HL>;
	u[0x2C] = &Cpu::inc_r8<Registers::L>;
	u[0x2D] = &Cpu::dec_r8<Registers::L>;
	u[0x2E] = &Cpu::ld_r8_imm8<Registers::L>;
	u[0x2F] = &Cpu::cpl;
	u[0x30] = &Cpu::jr_cond_imm8<Condition::NotCarryFlag>;
	u[0x31] = &Cpu::ld_r16_imm16<Registers::SP>;
	u[0x32] = &Cpu::ld_d_hl_a;
	u[0x33] = &Cpu::inc_r16<Registers::SP>;
	u[0x34] = &Cpu::inc_r8_hl;
	u[0x35] = &Cpu::dec_r8_hl;
	u[0x36] = &Cpu::ld_hl_imm8;
	u[0x37] = &Cpu::scf;
	u[0x38] = &Cpu::jr_cond_imm8<Condition::CarryFlag>;
	u[0x39] = &Cpu::add_hl_r16<Registers::SP>;
	u[0x3A] = &Cpu::ld_d_a_hl;
	u[0x3B] = &Cpu::dec_r16<Registers::SP>;
	u[0x3C] = &Cpu::inc_r8<Registers::A>;
	u[0x3D] = &Cpu::dec_r8<Registers::A>;
	u[0x3E] = &Cpu::ld_r8_imm8<Registers::A>;
	u[0x3F] = &Cpu::ccf;
	u[0x40] = &Cpu::ld_r8_r8<Registers::B, Registers::B>;
	u[0x41] = &Cpu::ld_r8_r8<Registers::B, Registers::C>;
	u[0x42] = &Cpu::ld_r8_r8<Registers::B, Registers::D>;
	u[0x43] = &Cpu::ld_r8_r8<Registers::B, Registers::E>;
	u[0x44] = &Cpu::ld_r8_r8<Registers::B, Registers::H>;
	u[0x45] = &Cpu::ld_r8_r8<Registers::B, Registers::L>;
	u[0x46] = &Cpu::ld_r8_hl<Registers::B>;
	u[0x47] = &Cpu::ld_r8_r8<Registers::B, Registers::A>;
	u[0x48] = &Cpu::ld_r8_r8<Registers::C, Registers::B>;
	u[0x49] = &Cpu::ld_r8_r8<Registers::C, Registers::C>;
	u[0x4A] = &Cpu::ld_r8_r8<Registers::C, Registers::D>;
	u[0x4B] = &Cpu::ld_r8_r8<Registers::C, Registers::E>;
	u[0x4C] = &Cpu::ld_r8_r8<Registers::C, Registers::H>;
	u[0x4D] = &Cpu::ld_r8_r8<Registers::C, Registers::L>;
	u[0x4E] = &Cpu::ld_r8_hl<Registers::C>;
	u[0x4F] = &Cpu::ld_r8_r8<Registers::C, Registers::A>;
	u[0x50] = &Cpu::ld_r8_r8<Registers::D, Registers::B>;
	u[0x51] = &Cpu::ld_r8_r8<Registers::D, Registers::C>;
	u[0x52] = &Cpu::ld_r8_r8<Registers::D, Registers::D>;
	u[0x53] = &Cpu::ld_r8_r8<Registers::D, Registers::E>;
	u[0x54] = &Cpu::ld_r8_r8<Registers::D, Registers::H>;
	u[0x55] = &Cpu::ld_r8_r8<Registers::D, Registers::L>;
	u[0x56] = &Cpu::ld_r8_hl<Registers::D>;
	u[0x57] = &Cpu::ld_r8_r8<Registers::D, Registers::A>;
	u[0x58] = &Cpu::ld_r8_r8<Registers::E, Registers::B>;
	u[0x59] = &Cpu::ld_r8_r8<Registers::E, Registers::C>;
	u[0x5A] = &Cpu::ld_r8_r8<Registers::E, Registers::D>;
	u[0x5B] = &Cpu::ld_r8_r8<Registers::E, Registers::E>;
	u[0x5C] = &Cpu::ld_r8_r8<Registers::E, Registers::H>;
	u[0x5D] = &Cpu::ld_r8_r8<Registers::E, Registers::L>;
	u[0x5E] = &Cpu::ld_r8_hl<Registers::E>;
	u[0x5F] = &Cpu::ld_r8_r8<Registers::E, Registers::A>;
	u[0x60] = &Cpu::ld_r8_r8<Registers::H, Registers::B>;
	u[0x61] = &Cpu::ld_r8_r8<Registers::H, Registers::C>;
	u[0x62] = &Cpu::ld_r8_r8<Registers::H, Registers::D>;
	u[0x63] = &Cpu::ld_r8_r8<Registers::H, Registers::E>;
	u[0x64] = &Cpu::ld_r8_r8<Registers::H, Registers::H>;
	u[0x65] = &Cpu::ld_r8_r8<Registers::H, Registers::L>;
	u[0x66] = &Cpu::ld_r8_hl<Registers::H>;
	u[0x67] = &Cpu::ld_r8_r8<Registers::H, Registers::A>;
	u[0x68] = &Cpu::ld_r8_r8<Registers::L, Registers::B>;
	u[0x69] = &Cpu::ld_r8_r8<Registers::L, Registers::C>;
	u[0x6A] = &Cpu::ld_r8_r8<Registers::L, Registers::D>;
	u[0x6B] = &Cpu::ld_r8_r8<Registers::L, Registers::E>;
	u[0x6C] = &Cpu::ld_r8_r8<Registers::L, Registers::H>;
	u[0x6D] = &Cpu::ld_r8_r8<Registers::L, Registers::L>;
	u[0x6E] = &Cpu::ld_r8_hl<Registers::L>;
	u[0x6F] = &Cpu::ld_r8_r8<Registers::L, Registers::A>;
	u[0x70] = &Cpu::ld_hl_r8<Registers::B>;
	u[0x71] = &Cpu::ld_hl_r8<Registers::C>;
	u[0x72] = &Cpu::ld_hl_r8<Registers::D>;
	u[0x73] = &Cpu::ld_hl_r8<Registers::E>;
	u[0x74] = &Cpu::ld_hl_r8<Registers::H>;
	u[0x75] = &Cpu::ld_hl_r8<Registers::L>;
	u[0x76] = &Cpu::halt;
	u[0x77] = &Cpu::ld_hl_r8<Registers::A>;
	u[0x78] = &Cpu::ld_r8_r8<Registers::A, Registers::B>;
	u[0x79] = &Cpu::ld_r8_r8<Registers::A, Registers::C>;
	u[0x7A] = &Cpu::ld_r8_r8<Registers::A, Registers::D>;
	u[0x7B] = &Cpu::ld_r8_r8<Registers::A, Registers::E>;
	u[0x7C] = &Cpu::ld_r8_r8<Registers::A, Registers::H>;
	u[0x7D] = &Cpu::ld_r8_r8<Registers::A, Registers::L>;
	u[0x7E] = &Cpu::ld_r8_hl<Registers::A>;
	u[0x7F] = &Cpu::ld_r8_r8<Registers::A, Registers::A>;
	u[0x80] = &Cpu::add_a_r8<Registers::B>;
	u[0x81] = &Cpu::add_a_r8<Registers::C>;
	u[0x82] = &Cpu::add_a_r8<Registers::D>;
	u[0x83] = &Cpu::add_a_r8<Registers::E>;
	u[0x84] = &Cpu::add_a_r8<Registers::H>;
	u[0x85] = &Cpu::add_a_r8<Registers::L>;
	u[0x86] = &Cpu::add_a_r8_hl;
	u[0x87] = &Cpu::add_a_r8<Registers::A>;
	u[0x88] = &Cpu::adc_a_r8<Registers::B>;
	u[0x89] = &Cpu::adc_a_r8<Registers::C>;
	u[0x8A] = &Cpu::adc_a_r8<Registers::D>;
	u[0x8B] = &Cpu::adc_a_r8<Registers::E>;
	u[0x8C] = &Cpu::adc_a_r8<Registers::H>;
	u[0x8D] = &Cpu::adc_a_r8<Registers::L>;
	u[0x8E] = &Cpu::adc_a_r8_hl;
	u[0x8F] = &Cpu::adc_a_r8<Registers::A>;
	u[0x90] = &Cpu::sub_a_r8<Registers::B>;
	u[0x91] = &Cpu::sub_a_r8<Registers::C>;
	u[0x92] = &Cpu::sub_a_r8<Registers::D>;
	u[0x93] = &Cpu::sub_a_r8<Registers::E>;
	u[0x94] = &Cpu::sub_a_r8<Registers::H>;
	u[0x95] = &Cpu::sub_a_r8<Registers::L>;
	u[0x96] = &Cpu::sub_a_r8_hl;
	u[0x97] = &Cpu::sub_a_r8<Registers::A>;
	u[0x98] = &Cpu::sbc_a_r8<Registers::B>;
	u[0x99] = &Cpu::sbc_a_r8<Registers::C>;
	u[0x9A] = &Cpu::sbc_a_r8<Registers::D>;
	u[0x9B] = &Cpu::sbc_a_r8<Registers::E>;
	u[0x9C] = &Cpu::sbc_a_r8<Registers::H>;
	u[0x9D] = &Cpu::sbc_a_r8<Registers::L>;
	u[0x9E] = &Cpu::sbc_a_r8_hl;
	u[0x9F] = &Cpu::sbc_a_r8<Registers::A>;
	u[0xA0] = &Cpu::and_a_r8<Registers::B>;
	u[0xA1] = &Cpu::and_a_r8<Registers::C>;
	u[0xA2] = &Cpu::and_a_r8<Registers::D>;
	u[0xA3] = &Cpu::and_a_r8<Registers::E>;
	u[0xA4] = &Cpu::and_a_r8<Registers::H>;
	u[0xA5] = &Cpu::and_a_r8<Registers::L>;
	u[0xA6] = &Cpu::and_a_r8_hl;
	u[0xA7] = &Cpu::and_a_r8<Registers::A>;
	u[0xA8] = &Cpu::xor_a_r8<Registers::B>;
	u[0xA9] = &Cpu::xor_a_r8<Registers::C>;
	u[0xAA] = &Cpu::xor_a_r8<Registers::D>;
	u[0xAB] = &Cpu::xor_a_r8<Registers::E>;
	u[0xAC] = &Cpu::xor_a_r8<Registers::H>;
	u[0xAD] = &Cpu::xor_a_r8<Registers::L>;
	u[0xAE] = &Cpu::xor_a_r8_hl;
	u[0xAF] = &Cpu::xor_a_r8<Registers::A>;
	u[0xB0] = &Cpu::or_a_r8<Registers::B>;
	u[0xB1] = &Cpu::or_a_r8<Registers::C>;
	u[0xB2] = &Cpu::or_a_r8<Registers::D>;
	u[0xB3] = &Cpu::or_a_r8<Registers::E>;
	u[0xB4] = &Cpu::or_a_r8<Registers::H>;
	u[0xB5] = &Cpu::or_a_r8<Registers::L>;
	u[0xB6] = &Cpu::or_a_r8_hl;
	u[0xB7] = &Cpu::or_a_r8<Registers::A>;
	u[0xB8] = &Cpu::cp_a_r8<Registers::B>;
	u[0xB9] = &Cpu::cp_a_r8<Registers::C>;
	u[0xBA] = &Cpu::cp_a_r8<Registers::D>;
	u[0xBB] = &Cpu::cp_a_r8<Registers::E>;
	u[0xBC] = &Cpu::cp_a_r8<Registers::H>;
	u[0xBD] = &Cpu::cp_a_r8<Registers::L>;
	u[0xBE] = &Cpu::cp_a_r8_hl;
	u[0xBF] = &Cpu::cp_a_r8<Registers::A>;
	u[0xC0] = &Cpu::ret_nz;
	u[0xC1] = &Cpu::pop_r16stk<Registers::BC>;
	u[0xC2] = &Cpu::jp_cond_nz;
	u[0xC3] = &Cpu::jp_imm16;
	u[0xC4] = &Cpu::call_cond_nz;
	u[0xC5] = &Cpu::push_r16stk<Registers::BC>;
	u[0xC6] = &Cpu::add_a_imm8;
	u[0xC7] = &Cpu::rst_00;
	u[0xC8] = &Cpu::ret_z;
	u[0xC9] = &Cpu::ret;
	u[0xCA] = &Cpu::jp_cond_z;
	u[0xCB] = &Cpu::prefix;
	u[0xCC] = &Cpu::call_cond_z;
	u[0xCD] = &Cpu::call_imm16;
	u[0xCE] = &Cpu::adc_a_imm8;
	u[0xCF] = &Cpu::rst_08;
	u[0xD0] = &Cpu::ret_nc;
	u[0xD1] = &Cpu::pop_r16stk<Registers::DE>;
	u[0xD2] = &Cpu::jp_cond_nc;
	u[0xD3] = &Cpu::lockup<0xD3>;
	u[0xD4] = &Cpu::call_cond_nc;
	u[0xD5] = &Cpu::push_r16stk<Registers::DE>;
	u[0xD6] = &Cpu::sub_a_imm8;
	u[0xD7] = &Cpu::rst_10;
	u[0xD8] = &Cpu::ret_c;
	u[0xD9] = &Cpu::reti;
	u[0xDA] = &Cpu::jp_cond_c;
	u[0xDB] = &Cpu::lockup<0xDB>;
	u[0xDC] = &Cpu::call_cond_c;
	u[0xDD] = &Cpu::lockup<0xDD>;
	u[0xDE] = &Cpu::sbc_a_imm8;
	u[0xDF] = &Cpu::rst_18;
	u[0xE0] = &Cpu::ldh_imm8_a;
	u[0xE1] = &Cpu::pop_r16stk<Registers::HL>;
	u[0xE2] = &Cpu::ld_c_a;
	u[0xE3] = &Cpu::lockup<0xE3>;
	u[0xE4] = &Cpu::lockup<0xE4>;
	u[0xE5] = &Cpu::push_r16stk<Registers::HL>;
	u[0xE6] = &Cpu::and_a_imm8;
	u[0xE7] = &Cpu::rst_20;
	u[0xE8] = &Cpu::add_sp_imm8;
	u[0xE9] = &Cpu::jp_hl;
	u[0xEA] = &Cpu::ld_imm16_a;
	u[0xEB] = &Cpu::lockup<0xEB>;
	u[0xEC] = &Cpu::lockup<0xEC>;
	u[0xED] = &Cpu::lockup<0xED>;
	u[0xEE] = &Cpu::xor_a_imm8;
	u[0xEF] = &Cpu::rst_28;
	u[0xF0] = &Cpu::ldh_a_imm8;
	u[0xF1] = &Cpu::pop_r16stk<Registers::AF>;
	u[0xF2] = &Cpu::ld_a_c;
	u[0xF3] = &Cpu::di;
	u[0xF4] = &Cpu::lockup<0xF4>;
	u[0xF5] = &Cpu::push_r16stk<Registers::AF>;
	u[0xF6] = &Cpu::or_a_imm8;
	u[0xF7] = &Cpu::rst_30;
	u[0xF8] = &Cpu::ld_hl_sp_imm8;
	u[0xF9] = &Cpu::ld_sp_hl;
	u[0xFA] = &Cpu::ld_a_imm16;
	u[0xFB] = &Cpu::ei;
	u[0xFC] = &Cpu::lockup<0xFC>;
	u[0xFD] = &Cpu::lockup<0xFD>;
	u[0xFE] = &Cpu::cp_a_imm8;
	u[0xFF] = &Cpu::rst_38;

#define p instructions[instruction_list::Prefixed]
	p[0x00] = &Cpu::rlc_r8<Registers::B>;
	p[0x01] = &Cpu::rlc_r8<Registers::C>;
	p[0x02] = &Cpu::rlc_r8<Registers::D>;
	p[0x03] = &Cpu::rlc_r8<Registers::E>;
	p[0x04] = &Cpu::rlc_r8<Registers::H>;
	p[0x05] = &Cpu::rlc_r8<Registers::L>;
	p[0x06] = &Cpu::rlc_r8_hl;
	p[0x07] = &Cpu::rlc_r8<Registers::A>;
	p[0x08] = &Cpu::rrc_r8<Registers::B>;
	p[0x09] = &Cpu::rrc_r8<Registers::C>;
	p[0x0A] = &Cpu::rrc_r8<Registers::D>;
	p[0x0B] = &Cpu::rrc_r8<Registers::E>;
	p[0x0C] = &Cpu::rrc_r8<Registers::H>;
	p[0x0D] = &Cpu::rrc_r8<Registers::L>;
	p[0x0E] = &Cpu::rrc_r8_hl;
	p[0x0F] = &Cpu::rrc_r8<Registers::A>;
	p[0x10] = &Cpu::rl_r8<Registers::B>;
	p[0x11] = &Cpu::rl_r8<Registers::C>;
	p[0x12] = &Cpu::rl_r8<Registers::D>;
	p[0x13] = &Cpu::rl_r8<Registers::E>;
	p[0x14] = &Cpu::rl_r8<Registers::H>;
	p[0x15] = &Cpu::rl_r8<Registers::L>;
	p[0x16] = &Cpu::rl_r8_hl;
	p[0x17] = &Cpu::rl_r8<Registers::A>;
	p[0x18] = &Cpu::rr_r8<Registers::B>;
	p[0x19] = &Cpu::rr_r8<Registers::C>;
	p[0x1A] = &Cpu::rr_r8<Registers::D>;
	p[0x1B] = &Cpu::rr_r8<Registers::E>;
	p[0x1C] = &Cpu::rr_r8<Registers::H>;
	p[0x1D] = &Cpu::rr_r8<Registers::L>;
	p[0x1E] = &Cpu::rr_r8_hl;
	p[0x1F] = &Cpu::rr_r8<Registers::A>;
	p[0x20] = &Cpu::sla_r8<Registers::B>;
	p[0x21] = &Cpu::sla_r8<Registers::C>;
	p[0x22] = &Cpu::sla_r8<Registers::D>;
	p[0x23] = &Cpu::sla_r8<Registers::E>;
	p[0x24] = &Cpu::sla_r8<Registers::H>;
	p[0x25] = &Cpu::sla_r8<Registers::L>;
	p[0x26] = &Cpu::sla_r8_hl;
	p[0x27] = &Cpu::sla_r8<Registers::A>;
	p[0x28] = &Cpu::sra_r8<Registers::B>;
	p[0x29] = &Cpu::sra_r8<Registers::C>;
	p[0x2A] = &Cpu::sra_r8<Registers::D>;
	p[0x2B] = &Cpu::sra_r8<Registers::E>;
	p[0x2C] = &Cpu::sra_r8<Registers::H>;
	p[0x2D] = &Cpu::sra_r8<Registers::L>;
	p[0x2E] = &Cpu::sra_r8_hl;
	p[0x2F] = &Cpu::sra_r8<Registers::A>;
	p[0x30] = &Cpu::swap_r8<Registers::B>;
	p[0x31] = &Cpu::swap_r8<Registers::C>;
	p[0x32] = &Cpu::swap_r8<Registers::D>;
	p[0x33] = &Cpu::swap_r8<Registers::E>;
	p[0x34] = &Cpu::swap_r8<Registers::H>;
	p[0x35] = &Cpu::swap_r8<Registers::L>;
	p[0x36] = &Cpu::swap_r8_hl;
	p[0x37] = &Cpu::swap_r8<Registers::A>;
	p[0x38] = &Cpu::srl_r8<Registers::B>;
	p[0x39] = &Cpu::srl_r8<Registers::C>;
	p[0x3A] = &Cpu::srl_r8<Registers::D>;
	p[0x3B] = &Cpu::srl_r8<Registers::E>;
	p[0x3C] = &Cpu::srl_r8<Registers::H>;
	p[0x3D] = &Cpu::srl_r8<Registers::L>;
	p[0x3E] = &Cpu::srl_r8_hl;
	p[0x3F] = &Cpu::srl_r8<Registers::A>;
	p[0x40] = &Cpu::bit_b3_r8<0x40, Registers::B>;
	p[0x41] = &Cpu::bit_b3_r8<0x41, Registers::C>;
	p[0x42] = &Cpu::bit_b3_r8<0x42, Registers::D>;
	p[0x43] = &Cpu::bit_b3_r8<0x43, Registers::E>;
	p[0x44] = &Cpu::bit_b3_r8<0x44, Registers::H>;
	p[0x45] = &Cpu::bit_b3_r8<0x45, Registers::L>;
	p[0x46] = &Cpu::bit_b3_r8_hl<0x46>;
	p[0x47] = &Cpu::bit_b3_r8<0x47, Registers::A>;
	p[0x48] = &Cpu::bit_b3_r8<0x48, Registers::B>;
	p[0x49] = &Cpu::bit_b3_r8<0x49, Registers::C>;
	p[0x4A] = &Cpu::bit_b3_r8<0x4A, Registers::D>;
	p[0x4B] = &Cpu::bit_b3_r8<0x4B, Registers::E>;
	p[0x4C] = &Cpu::bit_b3_r8<0x4C, Registers::H>;
	p[0x4D] = &Cpu::bit_b3_r8<0x4D, Registers::L>;
	p[0x4E] = &Cpu::bit_b3_r8_hl<0x4E>;
	p[0x4F] = &Cpu::bit_b3_r8<0x4F, Registers::A>;
	p[0x50] = &Cpu::bit_b3_r8<0x50, Registers::B>;
	p[0x51] = &Cpu::bit_b3_r8<0x51, Registers::C>;
	p[0x52] = &Cpu::bit_b3_r8<0x52, Registers::D>;
	p[0x53] = &Cpu::bit_b3_r8<0x53, Registers::E>;
	p[0x54] = &Cpu::bit_b3_r8<0x54, Registers::H>;
	p[0x55] = &Cpu::bit_b3_r8<0x55, Registers::L>;
	p[0x56] = &Cpu::bit_b3_r8_hl<0x56>;
	p[0x57] = &Cpu::bit_b3_r8<0x57, Registers::A>;
	p[0x58] = &Cpu::bit_b3_r8<0x58, Registers::B>;
	p[0x59] = &Cpu::bit_b3_r8<0x59, Registers::C>;
	p[0x5A] = &Cpu::bit_b3_r8<0x5A, Registers::D>;
	p[0x5B] = &Cpu::bit_b3_r8<0x5B, Registers::E>;
	p[0x5C] = &Cpu::bit_b3_r8<0x5C, Registers::H>;
	p[0x5D] = &Cpu::bit_b3_r8<0x5D, Registers::L>;
	p[0x5E] = &Cpu::bit_b3_r8_hl<0x5E>;
	p[0x5F] = &Cpu::bit_b3_r8<0x5F, Registers::A>;
	p[0x60] = &Cpu::bit_b3_r8<0x60, Registers::B>;
	p[0x61] = &Cpu::bit_b3_r8<0x61, Registers::C>;
	p[0x62] = &Cpu::bit_b3_r8<0x62, Registers::D>;
	p[0x63] = &Cpu::bit_b3_r8<0x63, Registers::E>;
	p[0x64] = &Cpu::bit_b3_r8<0x64, Registers::H>;
	p[0x65] = &Cpu::bit_b3_r8<0x65, Registers::L>;
	p[0x66] = &Cpu::bit_b3_r8_hl<0x66>;
	p[0x67] = &Cpu::bit_b3_r8<0x67, Registers::A>;
	p[0x68] = &Cpu::bit_b3_r8<0x68, Registers::B>;
	p[0x69] = &Cpu::bit_b3_r8<0x69, Registers::C>;
	p[0x6A] = &Cpu::bit_b3_r8<0x6A, Registers::D>;
	p[0x6B] = &Cpu::bit_b3_r8<0x6B, Registers::E>;
	p[0x6C] = &Cpu::bit_b3_r8<0x6C, Registers::H>;
	p[0x6D] = &Cpu::bit_b3_r8<0x6D, Registers::L>;
	p[0x6E] = &Cpu::bit_b3_r8_hl<0x6E>;
	p[0x6F] = &Cpu::bit_b3_r8<0x6F, Registers::A>;
	p[0x70] = &Cpu::bit_b3_r8<0x70, Registers::B>;
	p[0x71] = &Cpu::bit_b3_r8<0x71, Registers::C>;
	p[0x72] = &Cpu::bit_b3_r8<0x72, Registers::D>;
	p[0x73] = &Cpu::bit_b3_r8<0x73, Registers::E>;
	p[0x74] = &Cpu::bit_b3_r8<0x74, Registers::H>;
	p[0x75] = &Cpu::bit_b3_r8<0x75, Registers::L>;
	p[0x76] = &Cpu::bit_b3_r8_hl<0x76>;
	p[0x77] = &Cpu::bit_b3_r8<0x77, Registers::A>;
	p[0x78] = &Cpu::bit_b3_r8<0x78, Registers::B>;
	p[0x79] = &Cpu::bit_b3_r8<0x79, Registers::C>;
	p[0x7A] = &Cpu::bit_b3_r8<0x7A, Registers::D>;
	p[0x7B] = &Cpu::bit_b3_r8<0x7B, Registers::E>;
	p[0x7C] = &Cpu::bit_b3_r8<0x7C, Registers::H>;
	p[0x7D] = &Cpu::bit_b3_r8<0x7D, Registers::L>;
	p[0x7E] = &Cpu::bit_b3_r8_hl<0x7E>;
	p[0x7F] = &Cpu::bit_b3_r8<0x7F, Registers::A>;
	p[0x80] = &Cpu::res_b3_r8<0x80, Registers::B>;
	p[0x81] = &Cpu::res_b3_r8<0x81, Registers::C>;
	p[0x82] = &Cpu::res_b3_r8<0x82, Registers::D>;
	p[0x83] = &Cpu::res_b3_r8<0x83, Registers::E>;
	p[0x84] = &Cpu::res_b3_r8<0x84, Registers::H>;
	p[0x85] = &Cpu::res_b3_r8<0x85, Registers::L>;
	p[0x86] = &Cpu::res_b3_r8_hl<0x86>;
	p[0x87] = &Cpu::res_b3_r8<0x87, Registers::A>;
	p[0x88] = &Cpu::res_b3_r8<0x88, Registers::B>;
	p[0x89] = &Cpu::res_b3_r8<0x89, Registers::C>;
	p[0x8A] = &Cpu::res_b3_r8<0x8A, Registers::D>;
	p[0x8B] = &Cpu::res_b3_r8<0x8B, Registers::E>;
	p[0x8C] = &Cpu::res_b3_r8<0x8C, Registers::H>;
	p[0x8D] = &Cpu::res_b3_r8<0x8D, Registers::L>;
	p[0x8E] = &Cpu::res_b3_r8_hl<0x8E>;
	p[0x8F] = &Cpu::res_b3_r8<0x8F, Registers::A>;
	p[0x90] = &Cpu::res_b3_r8<0x90, Registers::B>;
	p[0x91] = &Cpu::res_b3_r8<0x91, Registers::C>;
	p[0x92] = &Cpu::res_b3_r8<0x92, Registers::D>;
	p[0x93] = &Cpu::res_b3_r8<0x93, Registers::E>;
	p[0x94] = &Cpu::res_b3_r8<0x94, Registers::H>;
	p[0x95] = &Cpu::res_b3_r8<0x95, Registers::L>;
	p[0x96] = &Cpu::res_b3_r8_hl<0x96>;
	p[0x97] = &Cpu::res_b3_r8<0x97, Registers::A>;
	p[0x98] = &Cpu::res_b3_r8<0x98, Registers::B>;
	p[0x99] = &Cpu::res_b3_r8<0x99, Registers::C>;
	p[0x9A] = &Cpu::res_b3_r8<0x9A, Registers::D>;
	p[0x9B] = &Cpu::res_b3_r8<0x9B, Registers::E>;
	p[0x9C] = &Cpu::res_b3_r8<0x9C, Registers::H>;
	p[0x9D] = &Cpu::res_b3_r8<0x9D, Registers::L>;
	p[0x9E] = &Cpu::res_b3_r8_hl<0x9E>;
	p[0x9F] = &Cpu::res_b3_r8<0x9F, Registers::A>;
	p[0xA0] = &Cpu::res_b3_r8<0xA0, Registers::B>;
	p[0xA1] = &Cpu::res_b3_r8<0xA1, Registers::C>;
	p[0xA2] = &Cpu::res_b3_r8<0xA2, Registers::D>;
	p[0xA3] = &Cpu::res_b3_r8<0xA3, Registers::E>;
	p[0xA4] = &Cpu::res_b3_r8<0xA4, Registers::H>;
	p[0xA5] = &Cpu::res_b3_r8<0xA5, Registers::L>;
	p[0xA6] = &Cpu::res_b3_r8_hl<0xA6>;
	p[0xA7] = &Cpu::res_b3_r8<0xA7, Registers::A>;
	p[0xA8] = &Cpu::res_b3_r8<0xA8, Registers::B>;
	p[0xA9] = &Cpu::res_b3_r8<0xA9, Registers::C>;
	p[0xAA] = &Cpu::res_b3_r8<0xAA, Registers::D>;
	p[0xAB] = &Cpu::res_b3_r8<0xAB, Registers::E>;
	p[0xAC] = &Cpu::res_b3_r8<0xAC, Registers::H>;
	p[0xAD] = &Cpu::res_b3_r8<0xAD, Registers::L>;
	p[0xAE] = &Cpu::res_b3_r8_hl<0xAE>;
	p[0xAF] = &Cpu::res_b3_r8<0xAF, Registers::A>;
	p[0xB0] = &Cpu::res_b3_r8<0xB0, Registers::B>;
	p[0xB1] = &Cpu::res_b3_r8<0xB1, Registers::C>;
	p[0xB2] = &Cpu::res_b3_r8<0xB2, Registers::D>;
	p[0xB3] = &Cpu::res_b3_r8<0xB3, Registers::E>;
	p[0xB4] = &Cpu::res_b3_r8<0xB4, Registers::H>;
	p[0xB5] = &Cpu::res_b3_r8<0xB5, Registers::L>;
	p[0xB6] = &Cpu::res_b3_r8_hl<0xB6>;
	p[0xB7] = &Cpu::res_b3_r8<0xB7, Registers::A>;
	p[0xB8] = &Cpu::res_b3_r8<0xB8, Registers::B>;
	p[0xB9] = &Cpu::res_b3_r8<0xB9, Registers::C>;
	p[0xBA] = &Cpu::res_b3_r8<0xBA, Registers::D>;
	p[0xBB] = &Cpu::res_b3_r8<0xBB, Registers::E>;
	p[0xBC] = &Cpu::res_b3_r8<0xBC, Registers::H>;
	p[0xBD] = &Cpu::res_b3_r8<0xBD, Registers::L>;
	p[0xBE] = &Cpu::res_b3_r8_hl<0xBE>;
	p[0xBF] = &Cpu::res_b3_r8<0xBF, Registers::A>;
	p[0xC0] = &Cpu::set_b3_r8<0xC0, Registers::B>;
	p[0xC1] = &Cpu::set_b3_r8<0xC1, Registers::C>;
	p[0xC2] = &Cpu::set_b3_r8<0xC2, Registers::D>;
	p[0xC3] = &Cpu::set_b3_r8<0xC3, Registers::E>;
	p[0xC4] = &Cpu::set_b3_r8<0xC4, Registers::H>;
	p[0xC5] = &Cpu::set_b3_r8<0xC5, Registers::L>;
	p[0xC6] = &Cpu::set_b3_r8_hl<0xC6>;
	p[0xC7] = &Cpu::set_b3_r8<0xC7, Registers::A>;
	p[0xC8] = &Cpu::set_b3_r8<0xC8, Registers::B>;
	p[0xC9] = &Cpu::set_b3_r8<0xC9, Registers::C>;
	p[0xCA] = &Cpu::set_b3_r8<0xCA, Registers::D>;
	p[0xCB] = &Cpu::set_b3_r8<0xCB, Registers::E>;
	p[0xCC] = &Cpu::set_b3_r8<0xCC, Registers::H>;
	p[0xCD] = &Cpu::set_b3_r8<0xCD, Registers::L>;
	p[0xCE] = &Cpu::set_b3_r8_hl<0xCE>;
	p[0xCF] = &Cpu::set_b3_r8<0xCF, Registers::A>;
	p[0xD0] = &Cpu::set_b3_r8<0xD0, Registers::B>;
	p[0xD1] = &Cpu::set_b3_r8<0xD1, Registers::C>;
	p[0xD2] = &Cpu::set_b3_r8<0xD2, Registers::D>;
	p[0xD3] = &Cpu::set_b3_r8<0xD3, Registers::E>;
	p[0xD4] = &Cpu::set_b3_r8<0xD4, Registers::H>;
	p[0xD5] = &Cpu::set_b3_r8<0xD5, Registers::L>;
	p[0xD6] = &Cpu::set_b3_r8_hl<0xD6>;
	p[0xD7] = &Cpu::set_b3_r8<0xD7, Registers::A>;
	p[0xD8] = &Cpu::set_b3_r8<0xD8, Registers::B>;
	p[0xD9] = &Cpu::set_b3_r8<0xD9, Registers::C>;
	p[0xDA] = &Cpu::set_b3_r8<0xDA, Registers::D>;
	p[0xDB] = &Cpu::set_b3_r8<0xDB, Registers::E>;
	p[0xDC] = &Cpu::set_b3_r8<0xDC, Registers::H>;
	p[0xDD] = &Cpu::set_b3_r8<0xDD, Registers::L>;
	p[0xDE] = &Cpu::set_b3_r8_hl<0xDE>;
	p[0xDF] = &Cpu::set_b3_r8<0xDF, Registers::A>;
	p[0xE0] = &Cpu::set_b3_r8<0xE0, Registers::B>;
	p[0xE1] = &Cpu::set_b3_r8<0xE1, Registers::C>;
	p[0xE2] = &Cpu::set_b3_r8<0xE2, Registers::D>;
	p[0xE3] = &Cpu::set_b3_r8<0xE3, Registers::E>;
	p[0xE4] = &Cpu::set_b3_r8<0xE4, Registers::H>;
	p[0xE5] = &Cpu::set_b3_r8<0xE5, Registers::L>;
	p[0xE6] = &Cpu::set_b3_r8_hl<0xE6>;
	p[0xE7] = &Cpu::set_b3_r8<0xE7, Registers::A>;
	p[0xE8] = &Cpu::set_b3_r8<0xE8, Registers::B>;
	p[0xE9] = &Cpu::set_b3_r8<0xE9, Registers::C>;
	p[0xEA] = &Cpu::set_b3_r8<0xEA, Registers::D>;
	p[0xEB] = &Cpu::set_b3_r8<0xEB, Registers::E>;
	p[0xEC] = &Cpu::set_b3_r8<0xEC, Registers::H>;
	p[0xED] = &Cpu::set_b3_r8<0xED, Registers::L>;
	p[0xEE] = &Cpu::set_b3_r8_hl<0xEE>;
	p[0xEF] = &Cpu::set_b3_r8<0xEF, Registers::A>;
	p[0xF0] = &Cpu::set_b3_r8<0xF0, Registers::B>;
	p[0xF1] = &Cpu::set_b3_r8<0xF1, Registers::C>;
	p[0xF2] = &Cpu::set_b3_r8<0xF2, Registers::D>;
	p[0xF3] = &Cpu::set_b3_r8<0xF3, Registers::E>;
	p[0xF4] = &Cpu::set_b3_r8<0xF4, Registers::H>;
	p[0xF5] = &Cpu::set_b3_r8<0xF5, Registers::L>;
	p[0xF6] = &Cpu::set_b3_r8_hl<0xF6>;
	p[0xF7] = &Cpu::set_b3_r8<0xF7, Registers::A>;
	p[0xF8] = &Cpu::set_b3_r8<0xF8, Registers::B>;
	p[0xF9] = &Cpu::set_b3_r8<0xF9, Registers::C>;
	p[0xFA] = &Cpu::set_b3_r8<0xFA, Registers::D>;
	p[0xFB] = &Cpu::set_b3_r8<0xFB, Registers::E>;
	p[0xFC] = &Cpu::set_b3_r8<0xFC, Registers::H>;
	p[0xFD] = &Cpu::set_b3_r8<0xFD, Registers::L>;
	p[0xFE] = &Cpu::set_b3_r8_hl<0xFE>;
	p[0xFF] = &Cpu::set_b3_r8<0xFF, Registers::A>;
}
