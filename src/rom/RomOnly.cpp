#include "rom/RomOnly.hpp"
#include "rom/Rom.hpp"
#include <cstdio>
#include <iostream>

uint8_t RomOnly::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[1][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			return ram_banks[0][addr - 0xA000];
		}
		return 0xFF;
	default:
		std::cerr << "should not reach this" << std::endl;
		return 0xFF;
	}
}

void RomOnly::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x1FFF) {
			ram_enable = (val == 0x0A);
		}
		break;
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			ram_banks[0][addr - 0xA000] = val;
		}
		break;
	default:
		std::cerr << "should not reach this" << std::endl;
		break;
	}
}

void RomOnly::reset() {
	ram_enable = false;
	Rom::reset();
}
