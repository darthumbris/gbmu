#ifndef MCB3_HPP
#define MCB3_HPP

#include "Rom.hpp"
#include <cstdint>
#include <cstdio>

class MCB3 : public Rom {

	enum class rtc_registers { seconds = 0x08, minutes = 0x09, hours = 0x0A, days = 0x0B, flags = 0x0C };

private:
	uint8_t rom_bank = 1; // 0x00 - 0x7F
	int8_t ram_bank = 0;  // 0x00 - 0x03
	bool ram_timer_enable = false;
	bool has_rtc = false;

	// Clock counter registers (only if MBC3 has a timer?)
	uint8_t seconds = 0; // 0x08 //0-59
	uint8_t minutes = 0; // 0x09 //0-59
	uint8_t hours = 0;   // 0x0A //0-23
	uint16_t days = 0;   // 0-511 days
	uint8_t flags = 0;   // 0bCH00000D:  C carry and H halt and D is for day counter most significant bit

	uint8_t latched_seconds = 0; // 0x08
	uint8_t latched_minutes = 0; // 0x09
	uint8_t latched_hours = 0;   // 0x0A
	uint16_t latched_days = 0;
	uint8_t latched_flags = 0; // 0bCH00000D:  C carry and H halt and D is for day counter most significant bit
	uint8_t rtc = 0;
	uint8_t latch = 0;
	int32_t last_time = 0;
	int32_t last_time_cached = 0;

	time_t timer = 0;
	uint8_t rtc_counter = 0;

	void update_rtc();

public:
	MCB3(const std::string rom_path, RomHeader rheader, bool battery)
	    : Rom(rom_path, rheader, battery), has_rtc(rheader.has_timer()) {}

	virtual uint8_t read_u8(uint16_t addr);
	virtual void write_u8(uint16_t addr, uint8_t val);

	virtual void update_clock();

	virtual void save_ram();
	virtual void load_ram();

	virtual void reset();

	virtual void serialize(std::ofstream &f);
	virtual void deserialize(std::ifstream &f);
};

#endif