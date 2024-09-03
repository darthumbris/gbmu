#include "rom/MCB1M.hpp"
#include <iostream>

uint8_t MCB1M::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			return ram_banks[ram_bank][addr - 0xA000];
		}
	default:
		std::cerr << "should not reach this" << std::endl;
	}
	return 0xFF;
}

void MCB1M::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x1FFF) {
			ram_enable = (val == 0x0A);
		} else if (addr <= 0x2FFF) {
			rom_bank = (ram_bank & (1 << 8)) | val;
		} else if (addr <= 0x3FFF) {
			rom_bank = (ram_bank & 0xFF) | ((val & 1) << 8);
		}
		rom_bank &= (uint8_t)rom_banks.size() - 1;
		if (addr >= 0x6000 && addr <= 0x7FFF && (val == 0x00 || val == 0x01)) {
			rom_ram_mode = val == 0x01;
		}
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x0F * rom_ram_mode;
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
