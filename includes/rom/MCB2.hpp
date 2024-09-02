#ifndef MCB2_HPP
#define MCB2_HPP

#include "Rom.hpp"

constexpr std::uint16_t mask8{0b0000'0001'0000'0000}; // represents bit 8

class MCB2 : public Rom {
private:
	uint8_t rom_bank = 1; // 0x01 - 0x0F
	bool ram_enable = false;
	bool battery = false;

public:
	MCB2(const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader), battery(battery) {}
	virtual ~MCB2();
	
	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
	void save_ram();
	void load_ram();
};

#endif