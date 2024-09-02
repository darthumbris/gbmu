#ifndef MCB5_HPP
#define MCB5_HPP

#include "Rom.hpp"

class MCB5 : public Rom {
private:
	uint16_t rom_bank = 1; // 0x00 - 0x1FF
	uint8_t secondary_rom_bank = 0;
	uint8_t ram_bank = 0;
	bool ram_enable = false;
	bool battery = false;
	bool rumble = false;

public:
	MCB5(const std::string rom_path, RomHeader rheader, bool battery, bool rumble);
	virtual ~MCB5();
	
	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
	void save_ram();
	void load_ram();
};

#endif