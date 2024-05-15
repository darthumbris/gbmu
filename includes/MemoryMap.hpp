#ifndef MEMORYMAP_HPP
#define MEMORYMAP_HPP

#include <vector>
#include <array>
#include <cstdint>

// https://gbdev.io/pandocs/Memory_Map.html

using Mem16k = std::array<uint8_t, 16384>;
using Mem8k = std::array<uint8_t, 8192>;
using Mem4k = std::array<uint8_t, 4096>;

class MemoryMap
{
private:
    Mem16k rom;
    std::vector<Mem16k> rom_banks;
    std::array<Mem8k, 2> vram;     // 0 for GB and 0-1 for Cgb (switchable banks)
    std::vector<Mem8k> ext_ram;    // From cartridge, switchable bank if any //32K max
    std::array<Mem4k, 8> work_ram; // In CGB mode, switchable bank 1–7
    std::array<Mem4k, 8> echo_ram; //(mirror of C000–DDFF) use of this area is prohibited.
    std::array<uint8_t, 160> oam;  // 40 * 4 bytes
    std::array<uint8_t, 96> not_usable;
    std::array<uint8_t, 128> io_registers;
    std::array<uint8_t, 127> high_ram;
    uint8_t interrupt;

public:
    MemoryMap(/* args */);

    ~MemoryMap();

    uint8_t read_u8(uint16_t addr);
    uint16_t read_u16(uint16_t addr);
    void write_u8(uint16_t addr, uint8_t val);
    void write_u16(uint16_t addr, uint16_t val);
};

#endif
