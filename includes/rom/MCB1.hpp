#ifndef MCB1_HPP
#define MCB1_HPP

#include "Rom.hpp"

class MCB1 : public Rom {
private:
	uint8_t rom_bank = 1;
	uint8_t secondary_rom_bank = 0; // only needed for 1MiB or larger roms
	uint8_t ram_bank = 0;           // 0-3
	bool ram_enable = false;
	bool rom_ram_mode = false; // Not needed for (≤ 8 KiB RAM and ≤ 512 KiB ROM)

public:
	MCB1(const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader, battery) {}

	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
};

#endif