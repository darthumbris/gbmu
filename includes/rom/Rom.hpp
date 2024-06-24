#ifndef ROM_HPP
#define ROM_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "rom/RomHeader.hpp"

using Mem16k = std::array<uint8_t, 16384>;
using Mem8k = std::array<uint8_t, 8192>;

// enum CGB_FLAGS {
// 	Non_CGB_Mode,
// 	CGB_Enhanced,
// 	CGB_ONLY,
// 	PGB_Mode,
// };

// enum CartridgeType {
// 	ROM_ONLY,
// 	MBC1,
// 	MBC1_RAM,
// 	MBC1_RAM_BATTERY,
// 	MBC2,
// 	MBC2_BATTERY,
// 	ROM_RAM,
// 	ROM_RAM_BATTERY,
// 	MMM01,
// 	MMM01_RAM,
// 	MMM01_RAM_BATTERY,
// 	MBC3_TIMER_BATTERY,
// 	MBC3_TIMER_RAM_BATTERY,
// 	MBC3,
// 	MBC3_RAM,
// 	MBC3_RAM_BATTERY,
// 	MBC5,
// 	MBC5_RAM,
// 	MBC5_RAM_BATTERY,
// 	MBC5_RUMBLE,
// 	MBC5_RUMBLE_RAM,
// 	MBC5_RUMBLE_RAM_BATTERY,
// 	MBC6,
// 	MBC7_SENSOR_RUMBLE_RAM_BATTERY,
// 	POCKET_CAMERA,
// 	BANDAI_TAMA5,
// 	HuC3,
// 	HuC1_RAM_BATTERY,
// 	Invalid_Cartridge
// };

class Rom {
private:
	// uint32_t entry_point;         // 0x100-0x103
	// std::array<uint8_t, 48> logo; // 0x104-0x133
	// char _name[15];               // 0x134-0x142
	// CGB_FLAGS cgb_flag;           // 0x143
	// uint16_t license_code;        // 0x144-0x145
	// bool sgb_flag;                // 0x146
	// CartridgeType cartridge_type; // 0x147
	// uint16_t rom_size;            // 0x148
	// uint8_t ram_size;             // 0x149
	// uint8_t dest_code;            // 0x14A
	// uint8_t old_license_code;     // 0x14B
	// uint8_t version;              // 0x14C
	// uint8_t header_checksum;      // 0x14D
	// uint16_t global_checksum;     // 0x14E-0x14F

	
	// uint8_t rom_bank = 1;
	// uint8_t ram_bank = 0;
	// bool ram_enable = false    // bool rom_ram_mode = false;
	RomHeader header;
	bool cgb_on = false;

public:
	Rom(const std::string rom_path, RomHeader rheader);
	virtual ~Rom();

	template<typename MBC, typename ...Args>
    static std::unique_ptr<Rom> make(Args... args)
    {
        return (std::make_unique<MBC>(args...));
    }

	// void print_rom() const;
	virtual uint8_t read_u8(uint16_t addr) = 0;
	virtual void write_u8(uint16_t addr, uint8_t val) = 0;

	virtual void serialize(std::ofstream &f) = 0;
	virtual void deserialize(std::ifstream &f) = 0;

	std::string name() const {
		return header.name();
	}
	// CartridgeType get_rom_type() const {
	// 	return cartridge_type;
	// }
	inline bool cgb_mode() const {
		return cgb_on;
	}

	protected:
		std::vector<Mem16k> rom_banks{0};
		std::vector<Mem8k> ram_banks{0};
};


//TODO handle MCB1, MCB2, MCB3, MCB5

#endif