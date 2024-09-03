#include "rom/MCB5.hpp"
#include <cstring>
#include <iostream>

MCB5::MCB5(const std::string rom_path, RomHeader rheader, bool battery, bool rumble)
    : Rom(rom_path, rheader, battery), rumble(rumble) {
	ram_bank = 0;
	rom_bank = 1;
	secondary_rom_bank = 0;
	ram_enable = false;
	for (size_t i = 0; i < ram_banks.size(); i++) {
		for (size_t j = 0; j < ram_banks[i].size(); j++) {
			ram_banks[i][j] = 0xFF;
		}
	}
}

uint8_t MCB5::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			return ram_banks[ram_bank][addr - 0xA000];
		}
		return 0xFF;
	default:
		std::cerr << "should not reach this" << std::endl;
		return 0xFF;
	}
}

void MCB5::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x1FFF) {
			ram_enable = (val == 0x0A);
		} else if (addr <= 0x2FFF) {
			rom_bank = (ram_bank & (1 << 8)) | val; // 8 least significant bits
		} else if (addr <= 0x3FFF) {
			rom_bank = (ram_bank & 0xFF) | ((val & 1) << 8); // 9th bit
		}
		rom_bank &= (uint8_t)rom_banks.size() - 1;
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x0F;
			ram_bank &= (uint8_t)ram_banks.size() - 1;
		}
		break;
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			ram_banks[ram_bank][addr - 0xA000] = val;
		}
		break;
	default:
		std::cerr << "should not reach this" << std::endl;
		break;
	}
}
