#ifndef MCB2_HPP
#define MCB2_HPP

#include "rom/Rom.hpp"

constexpr std::uint16_t mask8{0b0000'0001'0000'0000}; // represents bit 8

class MCB2 : public Rom {
private:
    uint8_t rom_bank = 1; // 0x01 - 0x0F
	bool ram_enable = false;
    bool battery = false;

public:
    MCB2(const std::string rom_path, RomHeader rheader,bool battery) : Rom(rom_path, rheader), battery(battery) {}
    uint8_t read_u8(uint16_t addr);
    void write_u8(uint16_t addr, uint8_t val);

    void serialize(std::ofstream &f);
    void deserialize(std::ifstream &f);
};

#endif