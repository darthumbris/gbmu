#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include <cstddef>
#include <iostream>
#include <fstream>

MemoryMap::MemoryMap(const std::string path, Cpu *cpu) : cpu(cpu)
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
        if (!boot_rom_loaded)
        {
            return boot_rom[addr];
        }
        return rom[addr];
    case 0x0100 ... 0x3FFF:
        return rom[addr];
    case 0x4000 ... 0x7FFF:
        return rom_banks[0][addr - 0x4000]; // TODO check how to get what rom_bank
    case 0x8000 ... 0x9FFF:
        return cpu->get_ppu().read_u8_ppu(addr);
    case 0xA000 ... 0xBFFF:
        return ext_ram[0][addr - 0xA000]; // TODO check how to get what extram_bank
    case 0xC000 ... 0xDFFF:
        return work_ram[wram_bank_select()][addr - 0xC000];
    case 0xE000 ... 0xFDFF:
        return echo_ram[wram_bank_select()][addr - 0xE000];
    case 0xFE00 ... 0xFE9F:
        return cpu->get_ppu().read_oam(addr & 0xFF);
    case 0xFEA0 ... 0xFEFF:
        return not_usable[addr - 0xFEA0];
    case 0xFF00 ... 0xFF3F:
        if (addr == 0xFF00) {
            uint8_t joy;
            switch (joypad) {
                case 1: joy =  (0x0F); break;
                case 2: joy =  (0x0F); break;
                default: joy = (0xFF); break;
            }
            return joy;
        }
        return io_registers[addr - 0xFF00];
    case 0xFF40 ... 0xFF4F:
        return cpu->get_ppu().read_u8_ppu(addr);
    case 0xFF51 ... 0xFF7F:
        return cpu->get_ppu().read_u8_ppu(addr);
    case 0xFF50:
        return io_registers[(std::size_t)(0xFF50 - 0xFF00)];
    case 0xFF80 ... 0xFFFE:
        return high_ram[addr - 0xFF80];
    case 0xFFFF:
        return cpu->get_interrupt_enable();

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
        if (!boot_rom_loaded)
        {
            boot_rom[addr] = val;
            break;
        }
        rom[addr] = val;
        break;
    case 0x0100 ... 0x3FFF:
        rom[addr] = val;
        break;
    case 0x4000 ... 0x7FFF:
        rom_banks[0][addr - 0x4000] = val; // TODO check how to get what rom_bank
        break;
    case 0x8000 ... 0x9FFF:
        cpu->get_ppu().write_u8_ppu(addr, val);
        break;
    case 0xA000 ... 0xBFFF:
        ext_ram[wram_bank_select()][addr - 0xA000] = val;
        break;
    case 0xC000 ... 0xDFFF:
        work_ram[wram_bank_select()][addr - 0xC000] = val;
        break;
    case 0xE000 ... 0xFDFF:
        echo_ram[wram_bank_select()][addr - 0xE000] = val;
        break;
    case 0xFE00 ... 0xFE9F:
        cpu->get_ppu().write_oam(addr & 0xFF, val);
        break;
    case 0xFEA0 ... 0xFEFF:
        not_usable[addr - 0xFEA0] = val;
        break;
    case 0xFF00 ... 0xFF3F:
        if (addr == 0xFF00) {
            joypad = (val >> 4) & 3;
            break;
        }
        io_registers[addr - 0xFF00] = val;
        break;
    case 0xFF40 ... 0xFF4F:
        cpu->get_ppu().write_u8_ppu(addr, val);
        break;
    case 0xFF51 ... 0xFF7F:
        cpu->get_ppu().write_u8_ppu(addr, val);
        break;
    case 0xFF50:
        if (!boot_rom_loaded) {
            boot_rom_loaded = true;
        }
        io_registers[(std::size_t)(0xFF50 - 0xFF00)] = val;
        break;
    case 0xFF80 ... 0xFFFE:
        high_ram[addr - 0xFF80] = val;
        break;
    case 0xFFFF:
        cpu->set_interrupt_enable(val);
        break;

    default:
        break;
    }
}

INLINE_FN void MemoryMap::write_u16(uint16_t addr, uint16_t val)
{
    write_u8(addr, (uint8_t)(val & 0xFF));
    write_u8(addr + 1, (uint8_t)((val & 0xFF00) >> 8));
}
