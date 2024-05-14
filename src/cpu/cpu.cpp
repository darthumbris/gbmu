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
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x6D:
    case 0x6E:
    case 0x6F:
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
        ld_r8_r8();
        break;
    case 0x77:
        ld_r16_a();
        break;
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
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
