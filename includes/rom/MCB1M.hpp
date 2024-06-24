#ifndef MCB1M_HPP
#define MCB1M_HPP

#include "rom/Rom.hpp"

// TODO handle this one

class MCB1M : public Rom {
private:
	uint8_t rom_bank = 1;
	uint8_t ram_bank = 0;
	bool ram_enable = false;
	bool rom_ram_mode = false;
	bool battery = false;

public:
	const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader), battery(battery) {}
	uint8_t read_u8(uint16_t addr);
	void write_u8(uint16_t addr, uint8_t val);
};

#endif