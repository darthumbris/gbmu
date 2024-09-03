#include "rom/MCB2.hpp"
#include <cstdint>
#include <iostream>

uint8_t MCB2::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xA200:
		if (ram_enable) {
			return ram[addr - 0xA000];
		}
		return 0xFF;
	case 0xA201 ... 0xBFFF:
		return 0x00;
	default:
		std::cerr << "should not reach this" << std::endl;
		return 0xFF;
	}
}

void MCB2::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x3FFF) {
			if (addr & mask8) {
				rom_bank = (val ? val & 0x1F : 1);
				rom_bank &= (uint8_t)rom_banks.size() - 1;
			} else {
				ram_enable = (val & 0x0A);
			}
		}
		break;
	case 0xA000 ... 0xA200:
		if (ram_enable) {
			ram[addr - 0xA000] = val;
		}
		break;
	case 0xA201 ... 0xBFFF:
		break;
	default:
		std::cerr << "should not reach this" << std::endl;
		break;
	}
}
