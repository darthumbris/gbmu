#include "Cpu.hpp"
#include <iostream>
#include <math.h>
#include <typeinfo>
#include <fstream>
#include <stdexcept>

Cpu::Cpu(Decoder dec, const std::string path) : decoder(dec)
{
    u8_registers = {0};
    pc = 0;
    sp = 0;
    debug_count = 0;
    // pc = 0x100;
    mmap = MemoryMap(path);
}

Cpu::~Cpu()
{
}

uint16_t Cpu::get_16bitregister(Registers reg) const
{
    if (reg < 8) // 8bit register
    {
        return (u8_registers[reg]);
    }
    else // 16-bit register
    {
        if (reg == Registers::SP) {
            return sp;
        }
        return (u8_registers[reg - Registers::BC] << 8) + u8_registers[reg - Registers::BC + 1];
    }
}

uint8_t Cpu::get_register(Registers reg) const
{
    if (reg < 8) // 8bit register
    {
        return (u8_registers[reg]);
    }
    else // 16-bit register
    {
        throw std::runtime_error("getting register");
        // if (reg == Registers::SP) {
        //     return sp;
        // }
    }
}

uint8_t Cpu::get_flag(uint8_t flag) const
{
    return ((u8_registers[Registers::F] >> flag) & 1); //TODO check if this bit shift is correct
}

void Cpu::set_16bitregister(Registers reg, uint16_t val)
{
    if (reg < 8) //8bit register
    {
        u8_registers[reg] = (uint8_t)val; //TODO check bit shifting here
    }
    else // 16-bit register
    {
        if (reg == Registers::SP) {
            sp = val;
        }
        else {
            u8_registers[reg - Registers::BC] = (uint8_t)(val >> 8);
            u8_registers[reg - Registers::BC + 1] = (uint8_t)(val & 0xff);
        }
    }
}

void Cpu::set_register(Registers reg, uint8_t val)
{
    if (reg < 8) //8bit register
    {
        u8_registers[reg] = val;
    }
    else // 16-bit register
    {
        throw std::runtime_error("setting register");
        if (reg == Registers::SP) {
            sp = val;
        }
        else {
            u8_registers[reg - Registers::BC] = (uint8_t)(val >> 8);
            u8_registers[reg - Registers::BC + 1] = (uint8_t)(val & 0xff);
        }
    }
}

void Cpu::set_flag(uint8_t flag, uint8_t val)
{
    u8_registers[Registers::F] ^= ((-val) ^ u8_registers[Registers::F]) & (1U << flag);
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

void Cpu::tick()
{
    auto dec = get_instruction();
    if (debug_count < 33) {
    std::cout << "debug count: " << debug_count << "\t";
        debug_print(std::get<1>(dec), std::get<0>(dec));
    }
    if (debug_count < 2147483647) {
        debug_count += 1;
    }
    // if (pc <= 0x100) {
    // }
    // else {
    //     throw std::runtime_error("reached start of rom");
    // }

    execute_instruction(std::get<1>(dec), std::get<0>(dec), std::get<2>(dec));
    pc &= 0xFFFF;
    if (pc == 0x0100)
    {
        std::cout << "Start of ROM" << std::endl;
        mmap.bios_loaded = true;
        exit(1);
    }
}

void Cpu::debug_print(Instruction in, uint8_t opcode)
{
    std::cout << "[0x" << std::setfill('0') << std::setw(4) << std::hex << pc - 1;
    std::cout << "]  0x" << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << "\t";
    in.print_instruction();

    if (interrupts)
        std::cout << "interrupts: 0x" << std::setfill('0') << std::setw(4) << std::hex << interrupts << std::dec << std::endl;
}

std::tuple<uint8_t, Instruction, bool> Cpu::get_instruction()
{
    bool prefix = false;
    uint8_t opcode = mmap.read_u8(pc);
    Instruction instruction;
    pc += 1;
    if (opcode == 0xCB)
    {
        opcode = mmap.read_u8(pc);
        pc += 1;
        instruction = decoder.prefixed_instructions[opcode];
        prefix = true;
    }
    else
    {
        instruction = decoder.instructions[opcode];
    }
    return {opcode, instruction, prefix};
}

void Cpu::execute_instruction(Instruction in, uint8_t opcode, bool is_prefix_ins)
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
        rst_tg3(in);
        break;
    case "LD"_:
        ld(opcode, in.operands);
        break;
    case "INC"_:
        inc(opcode, in.operands[0]);
        break;
    case "DEC"_:
        dec(opcode, in.operands[0]);
        break;
    case "RLCA"_:
        rlca();
        break;
    case "RLA"_:
        rla();
        break;
    case "ADD"_:
        add(opcode, in.operands[1]);
        break;
    case "RRCA"_:
        rrca();
        break;
    case "STOP"_:
        stop();
        break;
    case "JR"_:
        jr(opcode);
        break;
    case "RRA"_:
        rra();
        break;
    case "DAA"_:
        daa();
        break;
    case "CPL"_:
        cpl();
        break;
    case "SCF"_:
        scf();
        break;
    case "CCF"_:
        ccf();
        break;
    case "HALT"_:
        halt();
        break;
    case "ADC"_:
        adc(opcode, in.operands[1]);
        break;
    case "SUB"_:
        sub(opcode, in.operands[1]);
        break;
    case "SBC"_:
        sbc(opcode, in.operands[1]);
        break;
    case "AND"_:
        and_(opcode, in.operands[1]);
        break;
    case "XOR"_:
        xor_(opcode, in.operands[1]);
        break;
    case "OR"_:
        or_(opcode, in.operands[1]);
        break;
    case "CP"_:
        cp_(opcode, in.operands[1]);
        break;
    case "RET"_:
        ret(opcode);
        break;
    case "POP"_:
        pop_r16stk(in.operands[0]);
        break;
    case "JP"_:
        jp(opcode);
        break;
    case "CALL"_:
        call(opcode);
        break;
    case "PUSH"_:
        push_r16stk(in.operands[0]);
        break;
    case "ILLEGAL_D3"_:
        lockup();
        break;
    case "RETI"_:
        reti();
        break;
    case "ILLEGAL_DB"_:
        lockup();
        break;
    case "ILLEGAL_DD"_:
        lockup();
        break;
    case "LDH"_:
        ldh(opcode);
        break;
    case "ILLEGAL_E3"_:
        lockup();
        break;
    case "ILLEGAL_E4"_:
        lockup();
        break;
    case "ILLEGAL_EB"_:
        lockup();
        break;
    case "ILLEGAL_EC"_:
        lockup();
        break;
    case "ILLEGAL_ED"_:
        lockup();
        break;
    case "DI"_:
        di();
        break;
    case "ILLEGAL_F4"_:
        lockup();
        break;
    case "EI"_:
        ei();
        break;
    case "ILLEGAL_FC"_:
        lockup();
        break;
    case "ILLEGAL_FD"_:
        lockup();
        break;

    default:
        break;
    }
}

void Cpu::prefix(Instruction in, uint8_t opcode)
{
    switch (string_hash(in.mnemonic))
    {
    case "RLC"_:
        rlc_r8(opcode, in.operands[0]);
        break;
    case "RRC"_:
        rrc_r8(opcode, in.operands[0]);
        break;
    case "RL"_:
        rl_r8(opcode, in.operands[0]);
        break;
    case "RR"_:
        rr_r8(opcode, in.operands[0]);
        break;
    case "SLA"_:
        sla_r8(opcode, in.operands[0]);
        break;
    case "SRA"_:
        sra_r8(opcode, in.operands[0]);
        break;
    case "SWAP"_:
        swap_r8(opcode, in.operands[0]);
        break;
    case "SRL"_:
        srl_r8(opcode, in.operands[0]);
        break;
    case "BIT"_:
        bit_b3_r8(opcode, in.operands[1]);
        break;
    case "RES"_:
        res_b3_r8(opcode, in.operands[1]);
        break;
    case "SET"_:
        set_b3_r8(opcode, in.operands[1]);
        break;
    default:
        unimplemented(opcode);
        break;
    }
}

void Cpu::unimplemented(uint8_t opcode) { std::cout << "unimplemented opcode: 0x" << std::setfill('0') << std::setw(2) << std::hex << opcode << std::dec << std::endl; }
void Cpu::lockup() {}
