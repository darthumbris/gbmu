#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include <cstddef>
#include <iostream>
#include <fstream>

MemoryMap::MemoryMap(const std::string path, Cpu *cpu) : cpu(cpu)
{
    std::vector<std::byte> data;
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (ifs.fail() || !ifs.good())
    {
        std::cerr << "Error opening file '" << path << "'" << std::endl;
        exit(1);
    }
    uint32_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
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
        rom_banks.push_back(Mem16k());
    }
    for (int k = 1; k < 128; k ++) {
        for (size_t j = 0; j < 16384; j++)
        {
            if (i >= data.size())
            {
                break;
            }
            rom_banks[k][j] = (uint8_t)data[i];
            i += 1;
        }
        rom_banks.push_back(Mem16k());
    }
    ext_ram.push_back(Mem8k());
    std::ifstream cgb("cgb_boot.bin", std::ios::binary | std::ios::ate);
    std::cout << "cgb_boot size: " << cgb.tellg() << std::endl;
    cgb.read(reinterpret_cast<char*>(&cgb_boot_rom), sizeof(cgb_boot_rom));
    cgb.close();

    //TODO handle different cartridge types
}

MemoryMap::~MemoryMap()
{
}

INLINE_FN uint8_t MemoryMap::read_u8(uint16_t addr)
{
    // std::cout << "trying to read addr: " << std::hex << (std::size_t)addr << std::dec << std::endl;
    switch (addr)
    {
        //TODO have a check if gb or cgb mode and then load correct boot rom (also gb is 256 and cgb is 2048 bytes)
    case 0x0000 ... 0x07FF:
        if (!boot_rom_loaded && addr <= 0xFF)
        {
            return gb_boot_rom[addr];
        }
        return rom[addr];
    case 0x0800 ... 0x3FFF:
        return rom[addr];
    case 0x4000 ... 0x7FFF:
        return rom_banks[rom_bank][addr - 0x4000];
    case 0x8000 ... 0x9FFF:
        return cpu->get_ppu().read_u8_ppu(addr);
    case 0xA000 ... 0xBFFF:
        // std::cout << "ram bank: " << (uint16_t)ram_bank << " wram_bank: " << (uint16_t)wram_bank_select() << std::endl;
        // return ext_ram[ram_bank][addr - 0xA000]; //TODO make sure ext_ram and ram_bank are correct()
        return ext_ram[0][addr - 0xA000];
    case 0xC000 ... 0xDFFF:
        if (addr <= 0xCFFF) {
            return work_ram[0][uint16_t(addr & 0x0FFF)];
        }
        else {
            return work_ram[1][uint16_t(addr & 0x0FFF)];
            //fix this return work_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF];
        }
    case 0xE000 ... 0xFDFF:
        if (addr <= 0xEFFF) {
            return echo_ram[0][uint16_t(addr & 0x0FFF)];
        }
        else {
            return echo_ram[1][uint16_t(addr & 0x0FFF)];
            //TODO fix this return echo_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF];
        }
    case 0xFE00 ... 0xFE9F:
        return cpu->get_ppu().read_oam(addr & 0xFF);
    case 0xFEA0 ... 0xFEFF:
        return not_usable[addr - 0xFEA0];
    case 0xFF00 ... 0xFF3F:
        if (addr == 0xFF00) {
            switch (joypad) {
                case 1: return joypad_buttons;
                case 2: return joypad_dpad;
                default: return 0xFF;
            }
        }
        if (addr == 0xFF04) {
            return cpu->get_timer_divider();
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
    switch (addr)
    {
    case 0x0000 ... 0x07FF:
    //TODO have a check if gb or cgb mode and then load correct boot rom (also gb is 256 and cgb is 2048 bytes)
        if (!boot_rom_loaded && addr <= 0xFF)
        {
            gb_boot_rom[addr] = val;
            break;
        }
        if (addr <= 0x1FFF)
        {
            if (val == 0x0A)
                ram_enable = true;
            else if (val == 0x00)
                ram_enable = false;
            break;
        }
        break;
    case 0x0800 ... 0x3FFF:
        if (addr <= 0x1FFF)
        {
            if (val == 0x0A)
                ram_enable = true;
            else if (val == 0x00)
                ram_enable = false;
        }
        else if (addr <= 0x2FFF) {
            rom_bank = (ram_bank & (1 << 8)) | val;
        }
        else if (addr <= 0x3FFF) {
            rom_bank = (ram_bank & 0xFF) | ((val & 1) << 8);
        }
        break;
    case 0x4000 ... 0x7FFF:        
        if (addr <= 0x5FFF)
            ram_bank = val & 0x0F;
        break;
    case 0x8000 ... 0x9FFF:
        cpu->get_ppu().write_u8_ppu(addr, val);
        break;
    case 0xA000 ... 0xBFFF:
        ext_ram[0][addr - 0xA000] = val;
        // ext_ram[ram_bank][addr - 0xA000] = val; //TODO fix this
        break;
    case 0xC000 ... 0xDFFF:
        if (addr <= 0xCFFF) {
            work_ram[0][uint16_t(addr & 0x0FFF)] = val;
        }
        else {
            work_ram[1][uint16_t(addr & 0x0FFF)] = val;
            //TODO fix this work_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF] = val;
        }
        break;
    case 0xE000 ... 0xFDFF:
        if (addr <= 0xEFFF) {
            echo_ram[0][uint16_t(addr & 0x0FFF)] = val;
        }
        else {
            echo_ram[1][uint16_t(addr & 0x0FFF)] = val;
            //TODO fix this echo_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF] = val;
        }
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
        if (addr == 0xFF0F) {
            cpu->overwrite_interrupt(val);
            break;
        }
        if (addr == 0xFF04) {
            cpu->reset_timer_divider();
            break;
        }
        io_registers[addr - 0xFF00] = val;
        break;
    case 0xFF40 ... 0xFF4F:
        cpu->get_ppu().write_u8_ppu(addr, val);
        break;
    case 0xFF51 ... 0xFF7F:
        cpu->get_ppu().write_u8_ppu(addr, val);
        // if (addr == 0xFF70) {
        //     std::cout << "hey" << std::endl;
        // }
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

uint8_t MemoryMap::wram_bank_select() {return cpu->get_ppu().read_u8_ppu(0xFF70);}
