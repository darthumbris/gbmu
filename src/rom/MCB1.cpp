#include "rom/MCB1.hpp"
#include "debug.hpp"
#include <cstdio>
#include <fstream>
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
		if (ram_enable) {
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

void MCB1::serialize(std::ofstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.write(reinterpret_cast<const char *>(&rom_bank), sizeof(rom_bank));
	f.write(reinterpret_cast<const char *>(&ram_bank), sizeof(ram_bank));
	f.write(reinterpret_cast<const char *>(&ram_enable), sizeof(ram_enable));
	DEBUG_MSG("done serializing rom");
}

void MCB1::deserialize(std::ifstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.read(reinterpret_cast<char *>(&rom_bank), sizeof(rom_bank));
	f.read(reinterpret_cast<char *>(&ram_bank), sizeof(ram_bank));
	f.read(reinterpret_cast<char *>(&ram_enable), sizeof(ram_enable));
	DEBUG_MSG("done deserializing rom");
}

void MCB1::save_ram() {
	if (battery) {
		Rom::save_ram();
	}
}

void MCB1::load_ram() {
	if (battery) {
		Rom::load_ram();
	}
}
