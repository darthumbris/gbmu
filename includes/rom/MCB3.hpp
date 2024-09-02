#ifndef MCB3_HPP
#define MCB3_HPP

#include "Rom.hpp"

// TODO handle the RTC
class MCB3 : public Rom {
private:
	uint8_t rom_bank = 1; // 0x00 - 0x7F
	uint8_t ram_bank = 0; // 0x00 - 0x03
	bool ram_enable = false;

	// Clock counter registers (only if MBC3 has a timer?)
	uint8_t seconds = 0;  // 0x08
	uint8_t minutes = 0;  // 0x09
	uint8_t hours = 0;    // 0x0A
	uint8_t day_l = 0;    // 0x0B Lower 8 bits of Day Counter ($00-$FF)
	uint8_t day_h = 0;    // 0x0C Upper 1 bit of Day Counter (bit 0), Carry Bit (bit 7), Halt Flag (bit 6)
	bool latched = false; //

public:
	MCB3(const std::string rom_path, RomHeader rheader, bool battery) : Rom(rom_path, rheader, battery){}
	virtual ~MCB3();

	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
};

#endif