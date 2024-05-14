#include "Cpu.hpp"
#include <iostream>

Cpu::Cpu(Decoder dec) : decoder(dec)
{
    registers = {};
    pc = 0;
}

Cpu::~Cpu()
{
}

uint16_t Cpu::get_register(Registers reg) const
{
    if (reg % 2 == 0 && reg < 8) // High register
    {
        return (registers[reg / 2] >> 8);
    }
    else if (reg < 8) // Low Register
    {
        return (registers[reg / 2] & 0xFF);
    }
    else // 16-bit register
    {
        return registers[reg - Registers::BC];
    }
}

uint8_t Cpu::get_register_bit(Registers reg, uint8_t bit_loc) const
{
    return (get_register(reg) >> bit_loc) & 1;
}

uint8_t Cpu::get_flag(uint8_t flag) const
{
    return get_register_bit(Registers::F, flag);
}

void Cpu::set_register(Registers reg, uint16_t val)
{
    if (reg % 2 == 0 && reg < 8) // High register
    {
        uint16_t cur = get_register(reg);
        registers[reg / 2] = (cur & 0x00FF | (val << 8));
    }
    else if (reg < 8) // Low Register
    {
        uint16_t cur = get_register(reg);
        registers[reg / 2] = (cur & 0xFF00 | val);
    }
    else // 16-bit register
    {
        registers[reg - Registers::BC] = val;
    }
}

void Cpu::set_register_bit(Registers reg, uint8_t bit_loc, uint8_t val)
{
    /*to get the new value: Xor the val with current register value
     * and then and compare with 1 (unsigned) bit shifted to the bit_loc wanted
     */
    uint8_t new_value = ((-val) ^ registers[reg]) & (1U << bit_loc);
    // This value is then Xor with the current register value so it only changes the value for the bit_loc
    registers[reg] ^= new_value;
}

void Cpu::set_flag(uint8_t flag, uint8_t val)
{
    set_register_bit(Registers::F, flag, val);
}

inline constexpr auto string_hash(const std::string_view sv)
{
    unsigned long hash{5381};
    for (unsigned char c : sv)
    {
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}

inline constexpr auto operator"" _(const char *str, size_t len)
{
    return string_hash(std::string_view{str, len});
}

void Cpu::execute_instruction(Instruction in, uint16_t opcode, bool is_prefix_ins)
{
    if (is_prefix_ins)
    {
        prefix(in, opcode);
        return;
    }
    if (in.mnemonic == "NOP")
        return;
    // TODO might also use the opcode itself? probably faster than checking the string_hash
    switch (string_hash(in.mnemonic))
    {
    case "RST"_:
        rst_tg3();
        break;
    case "STOP"_:
        stop();
        break;
    case "HALT"_:
        halt();
        break;
    case "EI"_:
        ei();
        break;
    case "DI"_:
        di();
        break;
    case "LD"_:
        ld(opcode);
        break;
    case "INC"_:
        inc(opcode);
        break;
    case "DEC"_:
        dec(opcode);
        break;

    case "RLCA"_:
        rlca();
        break;
    case "RLA"_:
        rla();
        break;

    case "RRCA"_:
        rrca();
        break;
    case "RRA"_:
        rra();
        break;
    default:
        break;
    }
}

void Cpu::tick()
{
    uint16_t opcode = pc;
    auto dec = decoder.decode(opcode);
    std::cout << "opcode: 0x" << std::setfill('0') << std::setw(4) << std::hex << opcode << std::dec << std::endl;
    pc = std::get<0>(dec);
    std::get<1>(dec).print_instruction();
    execute_instruction(std::get<1>(dec), pc, std::get<2>(dec));
}

void Cpu::ld(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x01:
        ld_r16_imm16();
        break;
    case 0x02:
        ld_r16_a();
        break;
    case 0x06:
        ld_r8_imm8();
        break;
    case 0x08:
        ld_imm16_sp();
        break;
    case 0x0A:
        ld_a_r16();
        break;
    case 0x0E:
        ld_r8_imm8();
        break;
    case 0x11:
        ld_r16_imm16();
        break;
    case 0x12:
        ld_r16_a();
        break;
    case 0x16:
        ld_r8_imm8();
        break;
    case 0x1A:
        ld_a_r16();
        break;
    case 0x1E:
        ld_r8_imm8();
        break;
    case 0x21:
        ld_r16_imm16();
        break;
    case 0x22:
        ld_increment_hl();
        break;
    case 0x26:
        ld_r8_imm8();
        break;
    case 0x2A:
        ld_increment_hl();
        break;
    case 0x2E:
        ld_r8_imm8();
        break;
    case 0x31:
        ld_r16_imm16();
        break;
    case 0x32:
        ld_decrement_hl();
        break;
    case 0x36:
        ld_hl_imm8();
        break;
    case 0x3A:
        ld_decrement_hl();
        break;
    case 0x3E:
        ld_r8_imm8();
        break;
    case 0x40 ... 0x75:
        ld_r8_r8();
        break;
    case 0x77:
        ld_r16_a();
        break;
    case 0x78 ... 0x7D:
        ld_r8_r8();
        break;
    case 0x7E:
        ld_a_r16();
        break;
    case 0x7F:
        ld_r8_r8();
        break;
    case 0xE2:
        ld_c_a();
        break;
    case 0xEA:
        ld_imm16_a();
        break;
    case 0xF2:
        ld_a_c();
        break;
    case 0xF8:
        ld_hl_sp_imm8();
        break;
    case 0xF9:
        ld_sp_hl();
        break;
    case 0xFA:
        ld_a_imm16();
        break;

    default:
        unimplemented();
        break;
    }
}

void Cpu::inc(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x03:
        inc_r16();
        break;
    case 0x04:
        inc_r8();
        break;
    case 0x0C:
        inc_r8();
        break;
    case 0x13:
        inc_r16();
        break;
    case 0x14:
        inc_r8();
        break;
    case 0x1C:
        inc_r8();
        break;
    case 0x23:
        inc_r16();
        break;
    case 0x24:
        inc_r8();
        break;
    case 0x2C:
        inc_r8();
        break;
    case 0x33:
        inc_r16();
        break;
    case 0x34:
        inc_r8();
        break;
    case 0x3C:
        inc_r8();
        break;

    default:
        unimplemented();
        break;
    }
}

void Cpu::dec(uint16_t opcode)
{
    switch (opcode)
    {
    case 0x05:
        dec_r8();
        break;
    case 0x0B:
        dec_r16();
        break;
    case 0x0D:
        dec_r8();
        break;
    case 0x15:
        dec_r8();
        break;
    case 0x1B:
        dec_r16();
        break;
    case 0x1D:
        dec_r8();
        break;
    case 0x25:
        dec_r8();
        break;
    case 0x2B:
        dec_r16();
        break;
    case 0x2D:
        dec_r8();
        break;
    case 0x35:
        dec_r8();
        break;
    case 0x3B:
        dec_r16();
        break;
    case 0x3D:
        dec_r8();
        break;

    default:
        unimplemented();
        break;
    }
}

void Cpu::prefix(Instruction in, uint16_t opcode)
{
    switch (string_hash(in.mnemonic))
    {
    case "RLC"_:
        rlc_r8();
        break;
    case "RL"_:
        rl_r8();
        break;
    case "SLA"_:
        sla_r8();
        break;
    case "RRC"_:
        rrc_r8();
        break;
    case "RR"_:
        rr_r8();
        break;
    case "SRA"_:
        sra_r8();
        break;
    case "SRL"_:
        srl_r8();
        break;
    case "SWAP"_:
        swap_r8();
        break;
    case "BIT"_:
        bit_b3_r8();
        break;
    case "RES"_:
        res_b3_r8();
        break;
    case "SET"_:
        set_b3_r8();
        break;
    default:
        break;
    }
}

void Cpu::rlc_r8() {}
void Cpu::rrc_r8() {}
void Cpu::rl_r8() {}
void Cpu::rr_r8() {}
void Cpu::sla_r8() {}
void Cpu::sra_r8() {}
void Cpu::swap_r8() {}
void Cpu::srl_r8() {}
void Cpu::bit_b3_r8() {}
void Cpu::res_b3_r8() {}
void Cpu::set_b3_r8() {}
void Cpu::ld_increment_hl() {}
void Cpu::ld_decrement_hl() {}
void Cpu::ld_hl_imm8() {}
void Cpu::ld_c_a() {}
void Cpu::ld_a_c() {}

void Cpu::unimplemented() {}
void Cpu::lockup() {}
void Cpu::nop() {}
void Cpu::ld_r16_imm16() {}
void Cpu::ld_r16_a() {}
void Cpu::ld_a_r16() {}
void Cpu::ld_imm16_sp() {}
void Cpu::inc_r16() {}
void Cpu::dec_r16() {}
void Cpu::add_hl_r16() {}
void Cpu::inc_r8() {}
void Cpu::dec_r8() {}
void Cpu::ld_r8_imm8() {}
void Cpu::rlca() {}
void Cpu::rrca() {}
void Cpu::rla() {}
void Cpu::rra() {}
void Cpu::daa() {}
void Cpu::cpl() {}
void Cpu::scf() {}
void Cpu::ccf() {}
void Cpu::jr_imm8() {}
void Cpu::jr_cond_imm8() {}
void Cpu::stop() {}
void Cpu::ld_r8_r8() {}
void Cpu::halt() {}
void Cpu::add_a_r8() {}
void Cpu::adc_a_r8() {}
void Cpu::sub_a_r8() {}
void Cpu::sbc_a_r8() {}
void Cpu::and_a_r8() {}
void Cpu::xor_a_r8() {}
void Cpu::or_a_r8() {}
void Cpu::cp_a_r8() {}
void Cpu::add_a_imm8() {}
void Cpu::adc_a_imm8() {}
void Cpu::sub_a_imm8() {}
void Cpu::sbc_a_imm8() {}
void Cpu::and_a_imm8() {}
void Cpu::xor_a_imm8() {}
void Cpu::or_a_imm8() {}
void Cpu::cp_a_imm8() {}
void Cpu::ret_cond() {}
void Cpu::ret() {}
void Cpu::reti() {}
void Cpu::jp_cond_imm16() {}
void Cpu::jp_imm16() {}
void Cpu::jp_hl() {}
void Cpu::call_cond_imm16() {}
void Cpu::call_imm16() {}
void Cpu::rst_tg3() {}
void Cpu::pop_r16stk() {}
void Cpu::push_r16stk() {}
void Cpu::ldh_c_a() {}
void Cpu::ldh_imm8_a() {}
void Cpu::ld_imm16_a() {}
void Cpu::ldh_a_c() {}
void Cpu::ldh_a_imm8() {}
void Cpu::ld_a_imm16() {}
void Cpu::add_sp_imm8() {}
void Cpu::ld_hl_sp_imm8() {}
void Cpu::ld_sp_hl() {}
void Cpu::di() {}
void Cpu::ei() {}