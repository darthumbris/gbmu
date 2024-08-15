#include "rom/RomOnly.hpp"
#include "debug.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>

uint8_t RomOnly::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[1][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable) {
			return ram_banks[0][addr - 0xA000];
		}
		return 0xFF;
	default:
		std::cout << "should not reach this" << std::endl;
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
			DEBUG_MSG("writing at addr: %#06x ram_bank: 0 ram_banks size: %zu\n", addr, ram_banks.size());
			ram_banks[0][addr - 0xA000] = val;
		}
		break;
	default:
		std::cout << "should not reach this" << std::endl;
		break;
	}
}

void RomOnly::serialize(std::ofstream &f) {
	for (int i = 0; i < rom_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < ram_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.write(reinterpret_cast<const char *>(&ram_enable), sizeof(ram_enable));
	std::cout << "done serializing rom" << std::endl;
}

void RomOnly::deserialize(std::ifstream &f) {
	for (int i = 0; i < rom_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.read(reinterpret_cast<char *>(&ram_enable), sizeof(ram_enable));
	std::cout << "done deserializing rom" << std::endl;
}

void RomOnly::save_ram() {
	if (battery) {
		Rom::save_ram();
	}
}

void RomOnly::load_ram() {
	if (battery) {
		Rom::load_ram();
	}
}
