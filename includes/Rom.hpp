#ifndef ROM_HPP
#define ROM_HPP

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <iostream>

using Mem16k = std::array<uint8_t, 16384>;
using Mem8k = std::array<uint8_t, 8192>;

enum CGB_FLAGS
{
    Non_CGB_Mode,
    CGB_Enhanced,
    CGB_ONLY,
    PGB_Mode,
};

enum CartridgeType {
    ROM_ONLY,
    MBC1,
    MBC1_RAM,
    MBC1_RAM_BATTERY,
    MBC2,
    MBC2_BATTERY,
    ROM_RAM,
    ROM_RAM_BATTERY,
    MMM01,
    MMM01_RAM,
    MMM01_RAM_BATTERY,
    MBC3_TIMER_BATTERY,
    MBC3_TIMER_RAM_BATTERY,
    MBC3,
    MBC3_RAM,
    MBC3_RAM_BATTERY,
    MBC5,
    MBC5_RAM,
    MBC5_RAM_BATTERY,
    MBC5_RUMBLE,
    MBC5_RUMBLE_RAM,
    MBC5_RUMBLE_RAM_BATTERY,
    MBC6,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY,
    POCKET_CAMERA,
    BANDAI_TAMA5,
    HuC3,
    HuC1_RAM_BATTERY,
    Invalid_Cartridge
};

class Rom
{
private:
    uint32_t entry_point;           // 0x100-0x103
    std::array<uint8_t, 48> logo;   // 0x104-0x133
    char _name[15];                 // 0x134-0x142
    CGB_FLAGS cgb_flag;             // 0x143
    uint16_t license_code;          // 0x144-0x145
    bool sgb_flag;                  // 0x146
    CartridgeType cartridge_type;   // 0x147
    uint16_t rom_size;               // 0x148
    uint8_t ram_size;               // 0x149
    uint8_t dest_code;              // 0x14A
    uint8_t old_license_code;       // 0x14B
    uint8_t version;                // 0x14C
    uint8_t header_checksum;        // 0x14D
    uint16_t global_checksum;       // 0x14E-0x14F

    std::vector<Mem16k> rom_banks{0};
    std::vector<Mem8k> ram_banks{0};
    uint8_t rom_bank = 1;
    uint8_t ram_bank = 0;
    bool ram_enable = false;

public:
    Rom(const std::string path);
    ~Rom();

    void print_rom() const;
    uint8_t read_u8(uint16_t addr);
    uint16_t read_u16(uint16_t addr);
    void write_u8(uint16_t addr, uint8_t val);
    void write_u16(uint16_t addr, uint16_t val);

    void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f);
};

#endif