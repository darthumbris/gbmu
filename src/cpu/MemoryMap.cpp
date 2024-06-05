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
    // std::cout << "memory map rom size: " << data.size() << std::endl;
    // std::cout << "mmap path: " << path << std::endl;
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

uint8_t MemoryMap::read_u8(uint16_t addr)
{
    // TODO check if the bitwise operators for the addresses are correct
    // std::cout << "trying to read addr: " << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
    case 0x0000 ... 0x00FF:
        if (!is_boot_rom_enabled())
        {
            // std::cout << "reading from bios" << std::endl;
            return boot_rom[addr];
        }
        // std::cout << "reading from rom" << std::endl;
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
        // std::cout << "reading from highram: " << std::hex << addr << ", val: "<< (uint16_t)high_ram[(std::size_t)(addr & 0x7E)] << std::dec << std::endl;
        // std::cout << "address: " << std::dec << (std::size_t)(addr & 0x7E) << std::endl;
        return high_ram[addr - 0xFF80];
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
    if (addr == 0xFF50) {
        // std::cout << "this is for disabling boot rom?" << std::endl;
        exit(1);
    }
    // TODO check if the bitwise operators for the addresses are correct
    // std::cout << "trying to write addr: 0x" << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
    case 0x0000 ... 0x00FF:
        if (!is_boot_rom_enabled())
        {
            // printf("trying to write to bios at addr: %lu bios is size: %lu\n", (std::size_t)(addr), boot_rom.size());
            // std::cout << "bios trying to write addr: " << st::dec << (std::size_t)(addr) << std::dec << std::endl;
            // std::cout << "writing to bios" << std::endl;
            boot_rom[addr] = val;
            break;
        }
        // std::cout << "rom trying to write addr: " << std::dec << (std::size_t)(addr) << std::dec << std::endl;
        rom[addr] = val;
        break;
    case 0x0100 ... 0x3FFF:
        // std::cout << "rom trying to write addr: " << std::dec << (std::size_t)(addr) << std::dec << std::endl;
        rom[addr] = val;
        break;
    case 0x4000 ... 0x7FFF:
        // std::cout << "rom[0] trying to write addr: " << std::dec << (std::size_t)(addr & 0x3FFF) << std::dec << std::endl;
        rom_banks[0][addr - 0x4000] = val; // TODO check how to get what rom_bank
        break;
    case 0x8000 ... 0x9FFF:
        // if (addr > 0x9800 && val)
        // std::cout << "vram["<< (uint16_t)vram_bank_select() <<"]: trying to write addr: " << std::hex << addr << std::dec << " val: " << (uint16_t)val << std::endl;
        vram[vram_bank_select()][addr - 0x8000] = val; // TODO check how to get what vram_bank
        break;
    case 0xA000 ... 0xBFFF:
        // std::cout << "ext[0] trying to write addr: " << std::dec << (std::size_t)(addr & 0x1FFF) << std::dec << std::endl;
        ext_ram[0][addr - 0xA000] = val; // TODO check how to get what extram_bank
        break;
    case 0xC000 ... 0xDFFF:
        // std::cout << "work[0] trying to write addr: " << std::dec << (std::size_t)(addr & 0x0FFF) << std::dec << std::endl;
        work_ram[wram_bank_select()][addr - 0xC000] = val;
        break;
    case 0xE000 ... 0xFDFF:
        // std::cout << "echo trying to write addr: " << std::dec << (std::size_t)(addr & 0x0FFF) << std::dec << std::endl;
        echo_ram[wram_bank_select()][addr - 0xE000] = val;
        break;
    case 0xFE00 ... 0xFE9F:
        // std::cout << "oam: trying to write addr: " << std::dec << (std::size_t)(addr & 0x9F) << std::dec << std::endl;
        oam[addr - 0xFE00] = val;
        break;
    case 0xFEA0 ... 0xFEFF:
        // std::cout << "Not usable trying to write addr: " << std::dec << (std::size_t)(addr & 0x5F) << std::dec << std::endl;
        not_usable[addr - 0xFEA0] = val;
        break;
    case 0xFF00 ... 0xFF7F:
        // std::cout << "io_registers trying to write addr: " << std::hex << addr << std::dec << " val: " << (uint16_t)val << std::endl;
        // std::cout << "io trying to write addr: 0x" << std::hex << (std::size_t)(addr) << std::dec << std::endl;
        io_registers[addr - 0xFF00] = val;
        break;
    case 0xFF80 ... 0xFFFE:
        // std::cout << "reading from highram: " << std::hex << addr << ", val: "<< (uint16_t)high_ram[(std::size_t)(addr & 0x7E)] << std::dec << std::endl;
        // printf("trying to write to high_ram at addr: %d\n", (std::size_t)(addr & 0x7E));
        // if (addr == 0xFFFa || addr == 0xFFFb) {
        // std::cout << "high ram trying to write addr: " << std::hex << addr << " with val: " << (uint16_t)val << std::dec << std::endl;
        // std::cout << "address: " << std::dec << (std::size_t)(addr & 0x7E) << std::endl;
        // std::cout << "address - thing: " << std::dec << addr - 0xFF80 << std::endl;
        // }
        high_ram[addr - 0xFF80] = val;
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
    std::cout << "getting sprite" << std::endl;
    return {y_pos, x_pos, tile_index, attr_flags};
}