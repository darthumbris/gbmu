#include "Cpu.hpp"
#include <iostream>
#include <math.h>
#include <typeinfo>
#include <fstream>
#include <stdexcept>

uint32_t const DEBUG_START = 0;
uint32_t const DEBUG_COUNT = 48187;

Cpu::Cpu(Decoder dec, const std::string path) : decoder(dec)
{
    u8_registers = {0};
    pc = 0;
    sp = 0;
    debug_count = 0;
    m_cycle = 0;
    t_cycle = 0;
    set_instructions();
    mmap = MemoryMap(path);
}

Cpu::~Cpu()
{
}

uint16_t Cpu::get_16bitregister(Registers reg) const
{
    if (reg < 8) // 8bit register
    {
        std::cout << "what" << std::endl;
        return (u8_registers[reg]);
    }
    else // 16-bit register
    {
        if (reg == Registers::SP) {
            return sp;
        }
        return ((uint16_t)(u8_registers[reg - Registers::BC] << 8) + (uint16_t)u8_registers[reg - Registers::BC + 1]);
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
        std::cout << "what" << std::endl;
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

void Cpu::tick()
{
    auto dec = get_instruction();
    // if (debug_count >= DEBUG_START && debug_count < DEBUG_START + DEBUG_COUNT) {
        // std::cout << debug_count << "  ";
        // std::cout << "debug count: " << debug_count << std::endl;
        // if (debug_count > 2218245 - 2 && debug_count < 2218245 + 2)
        //     debug_print(std::get<0>(dec), std::get<1>(dec));
        // printf("%d, registers b: %u, c: %u, d: %u, e: %u, h: %u, l: %u, a: %u, f: %u\n", debug_count, get_register(Registers::B), get_register(Registers::C), get_register(Registers::D), get_register(Registers::E), get_register(Registers::H), get_register(Registers::L), get_register(Registers::A), get_register(Registers::F));
    // }
    // else {
    // }
    // if (debug_count > 24578 && debug_count < 24630) {
        // std::bitset<8> x(get_register(Registers::F));
        // printf("%d, registers b: %u, c: %u, d: %u, e: %u, h: %u, l: %u, a: %u, f: %u\n", debug_count, get_register(Registers::B), get_register(Registers::C), get_register(Registers::D), get_register(Registers::E), get_register(Registers::H), get_register(Registers::L), get_register(Registers::A), get_register(Registers::F));
        // std::cout << "debug count: "  << debug_count << ", register: " << x << std::endl;
    // }
    execute_instruction(std::get<0>(dec), std::get<1>(dec));
    pc &= 0xFFFF;
    // if (pc == 0x0100)
    // {
        // std::cout << "Start of ROM" << std::endl;
        // mmap.bios_loaded = true;
        // exit(1);
    // }
    ppu.tick(t_cycle, mmap);
    event_handler();
    if (debug_count < 2147483647) {
        debug_count += 1;
    }
}

void Cpu::debug_print(uint8_t opcode, bool prefix)
{
    std::cout << "[0x" << std::setfill('0') << std::setw(4) << std::hex << pc;
    std::cout << "] 0x" << std::setfill('0') << std::setw(2) << (uint16_t)opcode << std::dec << "\t";
    std::cout << std::endl;

    if (prefix) {
        decoder.prefixed_instructions[opcode].print_instruction();
    }
    else {
        decoder.instructions[opcode].print_instruction();
    }

    // if (interrupts)
    //     std::cout << "interrupts: 0x" << std::setfill('0') << std::setw(4) << std::hex << interrupts << std::dec << std::endl;
}

std::tuple<uint8_t, bool> Cpu::get_instruction()
{
    bool prefix = false;
    uint8_t opcode = mmap.read_u8(pc);
    pc += 1;
    if (opcode == 0xCB)
    {
        opcode = mmap.read_u8(pc);
        pc += 1;
        prefix = true;
    }
    return {opcode, prefix};
}

void Cpu::execute_instruction(uint8_t opcode, bool is_prefix_ins)
{
    if (is_prefix_ins)
    {
        auto op = prefixed_instructions[opcode];
        (this->*op)();
    }
    else {
        auto op = unprefixed_instructions[opcode];
        (this->*op)();
    }
}

void Cpu::lockup() {}
