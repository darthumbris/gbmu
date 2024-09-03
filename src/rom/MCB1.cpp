#include "rom/MCB1.hpp"
#include <cstdio>
#include <iostream>

uint8_t MCB1::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		if (rom_ram_mode) {
			// DEBUG_MSG("rom_bank + secondary: %u\n", rom_bank + secondary_rom_bank);
			return rom_banks[0][addr];
		}
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		if (!rom_bank) {
			// DEBUG_MSG("rom_bank: %u rom_ram_mode %u addr: %#06x\n", rom_bank == 0 ? 1 : rom_bank, rom_ram_mode,
			// addr);
		}
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

void MCB1::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x1FFF) {
			ram_enable = (val == 0x0A);
		} else if (addr >= 0x2000 && addr <= 0x3FFF) {
			rom_bank = (val ? val & 0x1F : 1);
		}
		if (addr >= 0x6000 && addr <= 0x7FFF && (val == 0x00 || val == 0x01)) {
			rom_ram_mode = val == 0x01;
			// DEBUG_MSG("rom_ram_mode: %u\n", rom_ram_mode);
		}
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x03 * rom_ram_mode;
			secondary_rom_bank = (val & 0x04 * rom_ram_mode) << 6;
		}
		break;
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			// DEBUG_MSG("writing at addr: %#06x ram_bank: %u ram_banks size: %zu\n", addr, ram_bank, ram_banks.size());
			ram_banks[ram_bank][addr - 0xA000] = val;
		}
		break;
	default:
		std::cerr << "should not reach this" << std::endl;
		break;
	}
}
