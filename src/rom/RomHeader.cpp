#include "rom/RomHeader.hpp"
#include <cstdint>
#include <fstream>

template <typename IntegerType>
void bitsToInt(IntegerType &result, const char *bits, bool little_endian) {
	result = 0;
	if (little_endian) {
		for (int n = sizeof(result) - 1; n >= 0; n--) {
			result = (result << 8) + (uint8_t)bits[n];
		}
	} else {
		for (int n = 0; n < sizeof(result); n++) {
			result = (result << 8) + (uint8_t)bits[n];
		}
	}
}

CGB_FLAGS get_cgb_flag(uint8_t flag) {
	switch (flag) {
	case 0:
		return CGB_FLAGS::Non_CGB_Mode;
	case 0x80:
		return CGB_FLAGS::CGB_Enhanced;
	case 0xC0:
		return CGB_FLAGS::CGB_ONLY;
	default:
		if (flag & 0x80 && ((flag & 0x04) || (flag & 0x08))) {
			return CGB_FLAGS::PGB_Mode;
		}
		return CGB_FLAGS::Non_CGB_Mode;
	}
}

CartridgeType RomHeader::get_cartridge_type(uint8_t type, uint8_t rom_size) {
	if ((type != 0xEA) && (rom_size == 0))
        return ROM_ONLY;
	switch (type) {
	case 0x00:
		return ROM_ONLY;
	case 0x01:
		return MBC1;
	case 0x02:
		return MBC1_RAM;
	case 0x03:
		return MBC1_RAM_BATTERY;
	case 0x05:
		return MBC2;
	case 0x06:
		return MBC2_BATTERY;
	case 0x08:
		return ROM_RAM;
	case 0x09:
		return ROM_RAM_BATTERY;
	case 0x0B:
		return MMM01;
	case 0x0C:
		return MMM01_RAM;
	case 0x0D:
		return MMM01_RAM_BATTERY;
	case 0x0F:
		return MBC3_TIMER_BATTERY;
	case 0x10:
		return MBC3_TIMER_RAM_BATTERY;
	case 0x11:
		return MBC3;
	case 0x12:
		return MBC3_RAM;
	case 0x13:
		return MBC3_RAM_BATTERY;
	case 0x19:
		return MBC5;
	case 0x1A:
		return MBC5_RAM;
	case 0x1B:
		return MBC5_RAM_BATTERY;
	case 0x1C:
		return MBC5_RUMBLE;
	case 0x1D:
		return MBC5_RUMBLE_RAM;
	case 0x1E:
		return MBC5_RUMBLE_RAM_BATTERY;
	case 0x20:
		return MBC6;
	case 0x22:
		return MBC7_SENSOR_RUMBLE_RAM_BATTERY;
	case 0xFC:
		return POCKET_CAMERA;
	case 0xFD:
		return BANDAI_TAMA5;
	case 0xFE:
		return HuC3;
	case 0xFF:
		return HuC1_RAM_BATTERY;
	default:
		return Invalid_Cartridge;
	}
}

uint16_t get_rom_size(uint8_t s) {
	switch (s) {
	case 0x00:
		return 2;
	case 0x01:
		return 4;
	case 0x02:
		return 8;
	case 0x03:
		return 16;
	case 0x04:
		return 32;
	case 0x05:
		return 64;
	case 0x06:
		return 128;
	case 0x07:
		return 256;
	case 0x08:
		return 512;
	case 0x52:
		return 72;
	case 0x53:
		return 80;
	case 0x54:
		return 96;
	default:
		return 2;
	}
}

uint8_t get_ram_size(uint8_t s) {
	printf("ram_size: %u\n", s);
	switch (s) {
	case 0x00:
		return 0;
	case 0x01:
		return 0;
	case 0x02:
		return 1;
	case 0x03:
		return 4;
	case 0x04:
		return 16;
	case 0x05:
		return 8;
	default:
		return 0;
	}
}

std::string print_cgb_flag(CGB_FLAGS flag) {
	switch (flag) {
	case Non_CGB_Mode:
		return "Non_CGB_Mode";
	case CGB_Enhanced:
		return "CGB_Enhanced";
	case CGB_ONLY:
		return "CGB_ONLY";
	case PGB_Mode:
		return "PGB_Mode";
	default:
		return "Non_CGB_Mode";
	}
}

std::string print_cartridge_type(CartridgeType c) {
	switch (c) {
	case ROM_ONLY:
		return "ROM_ONLY";
	case MBC1:
		return "MBC1";
	case MBC1_RAM:
		return "MBC1_RAM";
	case MBC1_RAM_BATTERY:
		return "MBC1_RAM_BATTERY";
	case MBC2:
		return "MBC2";
	case MBC2_BATTERY:
		return "MBC2_BATTERY";
	case ROM_RAM:
		return "ROM_RAM";
	case ROM_RAM_BATTERY:
		return "ROM_RAM_BATTERY";
	case MMM01:
		return "MMM01";
	case MMM01_RAM:
		return "MMM01_RAM";
	case MMM01_RAM_BATTERY:
		return "MMM01_RAM_BATTERY";
	case MBC3_TIMER_BATTERY:
		return "MBC3_TIMER_BATTERY";
	case MBC3_TIMER_RAM_BATTERY:
		return "MBC3_TIMER_RAM_BATTERY";
	case MBC3:
		return "MBC3";
	case MBC3_RAM:
		return "MBC3_RAM";
	case MBC3_RAM_BATTERY:
		return "MBC3_RAM_BATTERY";
	case MBC5:
		return "MBC5";
	case MBC5_RAM:
		return "MBC5_RAM";
	case MBC5_RAM_BATTERY:
		return "MBC5_RAM_BATTERY";
	case MBC5_RUMBLE:
		return "MBC5_RUMBLE";
	case MBC5_RUMBLE_RAM:
		return "MBC5_RUMBLE_RAM";
	case MBC5_RUMBLE_RAM_BATTERY:
		return "MBC5_RUMBLE_RAM_BATTERY";
	case MBC6:
		return "MBC6";
	case MBC7_SENSOR_RUMBLE_RAM_BATTERY:
		return "MBC7_SENSOR_RUMBLE_RAM_BATTERY";
	case POCKET_CAMERA:
		return "POCKET_CAMERA";
	case BANDAI_TAMA5:
		return "BANDAI_TAMA5";
	case HuC3:
		return "HuC3";
	case HuC1_RAM_BATTERY:
		return "HuC1_RAM_BATTERY";
	default:
		return "Invalid_Cartridge";
	}
}

RomHeader::RomHeader(const std::string path) {
	std::ifstream ifs;
	ifs.open(path.c_str(), std::ifstream::binary);

	ifs.seekg(0x100);
	char vals[4];
	ifs.read(reinterpret_cast<char *>(&vals), sizeof(vals));
	bitsToInt(_entry_point, vals, true);
	ifs.read(reinterpret_cast<char *>(&_logo), sizeof(_logo));
	ifs.read(reinterpret_cast<char *>(&_name), sizeof(_name));
	_cgb_flag = get_cgb_flag(ifs.get());
	char v[2];
	ifs.read(reinterpret_cast<char *>(&v), sizeof(v));
	bitsToInt(_license_code, v, true);
	_sgb_flag = ifs.get() == 0x03;
	uint8_t type = ifs.get();
	uint8_t rom_size = ifs.get();
	_rom_size = get_rom_size(rom_size);
	_cartridge_type = get_cartridge_type(type, rom_size);
	_ram_size = get_ram_size(ifs.get());
	_dest_code = ifs.get();
	_old_license_code = ifs.get();
	_version = ifs.get();
	_header_checksum = ifs.get();
	char val[2];
	ifs.read(reinterpret_cast<char *>(&val), sizeof(val));
	bitsToInt(_global_checksum, val, true);
	ifs.close();
	print_rom_info();
}

RomHeader::~RomHeader() {}

void RomHeader::print_rom_info() const {
	printf("title: %s\n", _name);
	printf("cgb_flag: %s\n", print_cgb_flag(_cgb_flag).c_str());
	printf("license_code: %u\n", _license_code);
	printf("sgb_flag: %u\n", _sgb_flag);
	printf("cartridge_type: %s\n", print_cartridge_type(_cartridge_type).c_str());
	printf("rom_size: %u\n", _rom_size);
	printf("ram_size: %u\n", _ram_size);
	printf("dest_code: %u\n", _dest_code);
	printf("old_license_code: %u\n", _old_license_code);
	printf("version: %u\n", _version);
	printf("header_checksum: %u\n", _header_checksum);
	printf("global_checksum: %u\n", _global_checksum);
}

bool RomHeader::has_battery() const {
	return (_cartridge_type == MBC1_RAM_BATTERY || _cartridge_type == MBC2_BATTERY ||
	        _cartridge_type == ROM_RAM_BATTERY || _cartridge_type == MMM01_RAM_BATTERY ||
	        _cartridge_type == MBC3_TIMER_BATTERY || _cartridge_type == MBC3_TIMER_RAM_BATTERY ||
	        _cartridge_type == MBC3_RAM_BATTERY || _cartridge_type == MBC5_RAM_BATTERY ||
	        _cartridge_type == MBC5_RUMBLE_RAM_BATTERY || _cartridge_type == MBC7_SENSOR_RUMBLE_RAM_BATTERY ||
	        _cartridge_type == HuC1_RAM_BATTERY);
}

bool RomHeader::has_rumble() const {
	return (_cartridge_type == MBC5_RUMBLE || _cartridge_type == MBC5_RUMBLE_RAM ||
	        _cartridge_type == MBC5_RUMBLE_RAM_BATTERY || _cartridge_type == MBC7_SENSOR_RUMBLE_RAM_BATTERY);
}

bool RomHeader::has_timer() const {
	return (_cartridge_type == MBC3_TIMER_BATTERY || _cartridge_type == MBC3_TIMER_RAM_BATTERY);
}