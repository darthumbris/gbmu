#ifndef ROM_ONLY_HPP
#define ROM_ONLY_HPP

#include "Rom.hpp"

class RomOnly : public Rom {
private:
	bool ram_enable = false;

public:
	RomOnly(const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader, battery) {}

	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void reset();

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
};

#endif