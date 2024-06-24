#ifndef MCB2_HPP
#define MCB2_HPP

#include "rom/Rom.hpp"

class MCB2 : public Rom {
private:
    uint8_t rom_bank = 1;
	uint8_t ram_bank = 0;
	bool ram_enable = false;
    bool rom_ram_mode = false;
    bool battery = false;

public:
    MCB2(const std::string rom_path, bool battery) : Rom(rom_path), battery(battery) {}
    uint8_t read_u8(uint16_t addr);
    void write_u8(uint16_t addr, uint8_t val);
};

#endif