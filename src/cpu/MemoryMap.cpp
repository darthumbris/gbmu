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
    for (int i = 0; i < 144*160; i++) {
        framebuffer[i] = 0;
    }
}

MemoryMap::~MemoryMap()
{
}

INLINE_FN uint8_t MemoryMap::read_u8(uint16_t addr)
{
    // std::cout << "trying to read addr: " << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
    case 0x0000 ... 0x00FF:
        if (!is_boot_rom_enabled())
        {
            return boot_rom[addr];
        }
        std::cout << "reading from cartridge rom" << std::endl;
        return rom[addr];
    case 0x0100 ... 0x3FFF:
        return rom[addr];
    case 0x4000 ... 0x7FFF:
        return rom_banks[0][addr - 0x4000]; // TODO check how to get what rom_bank
    case 0x8000 ... 0x9FFF:
        return vram[vram_bank_select()][addr - 0x8000]; // TODO check how to get what vram_bank
    case 0xA000 ... 0xBFFF:
        return ext_ram[0][addr - 0xA000]; // TODO check how to get what extram_bank
    case 0xC000 ... 0xDFFF:
        return work_ram[wram_bank_select()][addr - 0xC000]; // TODO check how to get what workram_bank
    case 0xE000 ... 0xFDFF:
        return echo_ram[wram_bank_select()][addr - 0xE000]; // TODO check how to get what echo_ram_bank
    case 0xFE00 ... 0xFE9F:
        return oam[addr - 0xFE00];
    case 0xFEA0 ... 0xFEFF:
        return not_usable[addr - 0xFEA0];
    case 0xFF00 ... 0xFF7F:
        return io_registers[addr - 0xFF00];
    case 0xFF80 ... 0xFFFE:
        return high_ram[addr - 0xFF80];
    case 0xFFFF:
        return interrupt;

    default:
        return 0;
    }
}

INLINE_FN uint16_t MemoryMap::read_u16(uint16_t addr)
{
    return ((uint16_t)read_u8(addr) + ((uint16_t)read_u8(addr + 1) << 8));
}

INLINE_FN void MemoryMap::write_u8(uint16_t addr, uint8_t val)
{
    // std::cout << "trying to write addr: 0x" << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
    case 0x0000 ... 0x00FF:
        if (!is_boot_rom_enabled())
        {
            boot_rom[addr] = val;
            break;
        }
        std::cout << "writing to cartridge rom" << std::endl;
        rom[addr] = val;
        break;
    case 0x0100 ... 0x3FFF:
        rom[addr] = val;
        break;
    case 0x4000 ... 0x7FFF:
        rom_banks[0][addr - 0x4000] = val; // TODO check how to get what rom_bank
        break;
    case 0x8000 ... 0x9FFF:
        vram[vram_bank_select()][addr - 0x8000] = val; // TODO check how to get what vram_bank
        break;
    case 0xA000 ... 0xBFFF:
        ext_ram[0][addr - 0xA000] = val; // TODO check how to get what extram_bank
        break;
    case 0xC000 ... 0xDFFF:
        work_ram[wram_bank_select()][addr - 0xC000] = val;
        break;
    case 0xE000 ... 0xFDFF:
        echo_ram[wram_bank_select()][addr - 0xE000] = val;
        break;
    case 0xFE00 ... 0xFE9F:
        oam[addr - 0xFE00] = val;
        break;
    case 0xFEA0 ... 0xFEFF:
        not_usable[addr - 0xFEA0] = val;
        break;
    case 0xFF00 ... 0xFF7F:
        io_registers[addr - 0xFF00] = val;
        break;
    case 0xFF80 ... 0xFFFE:
        high_ram[addr - 0xFF80] = val;
        break;
    case 0xFFFF:
        interrupt = val;
        break;

    default:
        break;
    }
    if (addr == 0xFF50) {
        std::cout << "reached end of bios ops at val: " << (uint16_t)val << std::endl;
        std::cout << "is boot_rom_enabled: " << is_boot_rom_enabled() << std::endl;
        exit(1);
    }
}

INLINE_FN void MemoryMap::write_u16(uint16_t addr, uint16_t val)
{
    write_u8(addr, (uint8_t)(val & 0xFF));
    write_u8(addr + 1, (uint8_t)((val & 0xFF00) >> 8));
}

void MemoryMap::set_ppu_mode(uint8_t mode) {
    switch (mode) {
        case 0:
            io_registers[0xFF41 -0xFF00] &= ~(1 << 0);
            io_registers[0xFF41 -0xFF00] &= ~(1 << 1);
        break;
        case 1:
            io_registers[0xFF41 -0xFF00] |= 1 << 0;
            io_registers[0xFF41 -0xFF00] &= ~(1 << 1);
        break;
        case 2:
            io_registers[0xFF41 -0xFF00] &= ~(1 << 0);
            io_registers[0xFF41 -0xFF00] |= 1 << 1;
        break;
        case 3:
            io_registers[0xFF41 -0xFF00] |= 1 << 0;
            io_registers[0xFF41 -0xFF00] |= 1 << 1;
        break;
    }
}

Sprite MemoryMap::get_sprite(size_t index) {
    uint8_t y_pos = oam[index * 4];
    uint8_t x_pos = oam[index * 4 + 1];
    uint8_t tile_index = oam[index * 4 + 2];
    uint8_t attr_flags = oam[index * 4 + 3];
    return {y_pos, x_pos, tile_index, attr_flags};
}