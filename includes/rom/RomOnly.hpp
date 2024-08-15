#ifndef ROM_ONLY_HPP
#define ROM_ONLY_HPP

#include "Rom.hpp"

class RomOnly : public Rom {
private:
	bool ram_enable = false;
	bool battery = false;

public:
	RomOnly(const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader), battery(battery) {}

	uint8_t read_u8(uint16_t addr);
	void write_u8(uint16_t addr, uint8_t val);

	void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f);
	void save_ram();
	void load_ram();
};

#endif