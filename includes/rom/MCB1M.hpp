#ifndef MCB1M_HPP
#define MCB1M_HPP

#include "Rom.hpp"

class MCB1M : public Rom {
private:
	uint8_t rom_bank = 1;
	uint8_t ram_bank = 0;
	bool ram_enable = false;
	bool rom_ram_mode = false;

public:
	MCB1M(const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader, battery) {}

	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
};

#endif