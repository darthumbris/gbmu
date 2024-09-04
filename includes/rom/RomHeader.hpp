#ifndef ROM_HEADER_HPP
#define ROM_HEADER_HPP

#include <cstdint>
#include <string>

enum CGB_FLAGS {
	Non_CGB_Mode,
	CGB_Enhanced,
	CGB_ONLY,
	PGB_Mode,
};

enum CartridgeType {
	ROM_ONLY,
	ROM_RAM,
	ROM_RAM_BATTERY,
	MBC1,
	MBC1_RAM,
	MBC1_RAM_BATTERY,
	MBC2,
	MBC2_BATTERY,
	MMM01,
	MMM01_RAM,
	MMM01_RAM_BATTERY,
	MBC3_TIMER_BATTERY,
	MBC3_TIMER_RAM_BATTERY,
	MBC3,
	MBC3_RAM,
	MBC3_RAM_BATTERY,
	MBC5,
	MBC5_RAM,
	MBC5_RAM_BATTERY,
	MBC5_RUMBLE,
	MBC5_RUMBLE_RAM,
	MBC5_RUMBLE_RAM_BATTERY,
	MBC6,
	MBC7_SENSOR_RUMBLE_RAM_BATTERY,
	POCKET_CAMERA,
	BANDAI_TAMA5,
	HuC3,
	HuC1_RAM_BATTERY,
	Invalid_Cartridge
};

class RomHeader {
private:
	uint32_t _entry_point;                             // 0x100-0x103
	uint8_t _logo[48];                                 // 0x104-0x133
	char _name[15];                                    // 0x134-0x142
	CGB_FLAGS _cgb_flag = Non_CGB_Mode;                // 0x143
	uint16_t _license_code;                            // 0x144-0x145
	bool _sgb_flag = false;                            // 0x146
	CartridgeType _cartridge_type = Invalid_Cartridge; // 0x147
	uint16_t _rom_size = 0;                            // 0x148
	uint8_t _ram_size = 0;                             // 0x149
	uint8_t _dest_code;                                // 0x14A
	uint8_t _old_license_code;                         // 0x14B
	uint8_t _version;                                  // 0x14C
	uint8_t _header_checksum;                          // 0x14D
	uint16_t _global_checksum;                         // 0x14E-0x14F

	CartridgeType get_cartridge_type(uint8_t type, uint8_t rom_size);

public:
	RomHeader(const std::string rom_path);
	~RomHeader();

	void print_rom_info() const;
	std::string name() const {
		return _name;
	}
	CartridgeType cartridge_type() const {
		return _cartridge_type;
	}
	inline bool is_cgb_game() const {
		return (_cgb_flag == CGB_ONLY || _cgb_flag == CGB_Enhanced);
	}
	inline uint8_t ram_size() const {
		return _ram_size;
	}
	inline uint16_t rom_size() const {
		return _rom_size;
	}
	bool has_battery() const;
	bool has_rumble() const;
	bool has_timer() const;

	inline void disable_cgb_enhancement() {
		_cgb_flag = Non_CGB_Mode;
	}

	inline void force_cgb_enhancement() {
		_cgb_flag = CGB_Enhanced;
	}
};

#endif