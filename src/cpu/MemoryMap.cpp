#include "MemoryMap.hpp"
#include <cstddef>
#include <iostream>
#include <fstream>

MemoryMap::MemoryMap()
{
}

MemoryMap::MemoryMap(const std::string path)
{
    std::vector<std::byte> data;
    std::ifstream ifs;
    ifs.open(path.c_str(), std::ifstream::binary);
    auto size = ifs.tellg();
    while (!ifs.eof())
    {
        char c = ifs.get();
        data.push_back(static_cast<std::byte>(c));
    }
    ifs.close();
    std::cout << "memory map rom size: " << data.size() << std::endl;
    std::cout << "mmap path: " << path << std::endl;
    size_t i;
    for (i = 0; i < 16384; i++)
    {
        if (i >= data.size())
        {
            break;
        }
        rom[i] = (uint8_t)data[i];
    }
    if (i < data.size())
    {
        rom_banks.push_back(Mem16k());
    }
    for (size_t j = 0; j < 16384; j++)
    {
        if (i >= data.size())
        {
            break;
        }
        rom_banks[0][j] = (uint8_t)data[i];
        i += 1;
    }
    ext_ram.push_back(Mem8k());
}

MemoryMap::~MemoryMap()
{
}

uint8_t MemoryMap::read_u8(uint16_t addr)
{
    // TODO check if the bitwise operators for the addresses are correct
    // std::cout << "trying to read addr: " << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
    case 0x0000 ... 0x3FFF:
        if (!bios_loaded)
        {
            // std::cout << "reading from bios" << std::endl;
            return bios[(std::size_t)addr];
        }
        return rom[(std::size_t)addr];
    case 0x4000 ... 0x7FFF:
        return rom_banks[0][(std::size_t)(addr & 0x3FFF)]; // TODO check how to get what rom_bank
    case 0x8000 ... 0x9FFF:
        return vram[0][(std::size_t)(addr & 0x1FFF)]; // TODO check how to get what vram_bank
    case 0xA000 ... 0xBFFF:
        return ext_ram[0][(std::size_t)(addr & 0x1FFF)]; // TODO check how to get what extram_bank
    case 0xC000 ... 0xDFFF:
        return work_ram[0][(std::size_t)(addr & 0x0FFF)]; // TODO check how to get what workram_bank
    case 0xE000 ... 0xFDFF:
        return echo_ram[0][(std::size_t)(addr & 0x0FFF)]; // TODO check how to get what echo_ram_bank
    case 0xFE00 ... 0xFE9F:
        return oam[(std::size_t)(addr & 0x9F)];
    case 0xFEA0 ... 0xFEFF:
        return not_usable[(std::size_t)(addr & 0x5F)];
    case 0xFF00 ... 0xFF7F:
        return io_registers[(std::size_t)(addr & 0x7F)];
    case 0xFF80 ... 0xFFFE:
        return high_ram[(std::size_t)(addr & 0x7E)];
    case 0xFFFF:
        return interrupt;

    default:
        return 0;
    }
}
uint16_t MemoryMap::read_u16(uint16_t addr)
{
    return ((uint16_t)read_u8(addr) + ((uint16_t)read_u8(addr + 1) << 8));
}
void MemoryMap::write_u8(uint16_t addr, uint8_t val)
{
    // TODO check if the bitwise operators for the addresses are correct
    // std::cout << "trying to write addr: " << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
    case 0x0000 ... 0x3FFF:
        if (!bios_loaded)
        {
            // std::cout << "writing to bios" << std::endl;
            bios[(std::size_t)addr] = val;
        }
        rom[(std::size_t)addr] = val;
        break;
    case 0x4000 ... 0x7FFF:
        rom_banks[0][(std::size_t)(addr & 0x3FFF)] = val; // TODO check how to get what rom_bank
        break;
    case 0x8000 ... 0x9FFF:
        vram[0][(std::size_t)(addr & 0x1FFF)] = val; // TODO check how to get what vram_bank
        break;
    case 0xA000 ... 0xBFFF:
        ext_ram[0][(std::size_t)(addr & 0x1FFF)] = val; // TODO check how to get what extram_bank
        break;
    case 0xC000 ... 0xDFFF:
        work_ram[0][(std::size_t)(addr & 0x0FFF)] = val; // TODO check how to get what workram_bank
        break;
    case 0xE000 ... 0xFDFF:
        echo_ram[0][(std::size_t)(addr & 0x0FFF)] = val; // TODO check how to get what echo_ram_bank
        break;
    case 0xFE00 ... 0xFE9F:
        oam[(std::size_t)(addr & 0x9F)] = val;
        break;
    case 0xFEA0 ... 0xFEFF:
        not_usable[(std::size_t)(addr & 0x5F)] = val;
        break;
    case 0xFF00 ... 0xFF7F:
        io_registers[(std::size_t)(addr & 0x7F)] = val;
        break;
    case 0xFF80 ... 0xFFFE:
        high_ram[(std::size_t)(addr & 0x7E)] = val;
        break;
    case 0xFFFF:
        interrupt = val;
        break;

    default:
        break;
    }
}
void MemoryMap::write_u16(uint16_t addr, uint16_t val)
{
    write_u8(addr, (uint8_t)(val & 0xFF));
    write_u8(addr + 1, (uint8_t)((val & 0xFF00) >> 8));
}