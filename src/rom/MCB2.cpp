#include "rom/MCB2.hpp"
#include "debug.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>

//TODO check if rom_banks.size() check is needed

uint8_t MCB2::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
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

void MCB2::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x3FFF) {
			if (addr & mask8) {
				rom_bank = (val ? val & 0x1F : 1);
			} else {
				ram_enable = (val & 0x0A);
			}
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

void MCB2::serialize(std::ofstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.write(reinterpret_cast<const char *>(&rom_bank), sizeof(rom_bank));
	f.write(reinterpret_cast<const char *>(&ram_enable), sizeof(ram_enable));
	DEBUG_MSG("done serializing rom");
}

void MCB2::deserialize(std::ifstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.read(reinterpret_cast<char *>(&rom_bank), sizeof(rom_bank));
	f.read(reinterpret_cast<char *>(&ram_enable), sizeof(ram_enable));
	DEBUG_MSG("done deserializing rom");
}

void MCB2::save_ram() {
	if (battery) {
		Rom::save_ram();
	}
}

void MCB2::load_ram() {
	if (battery) {
		Rom::load_ram();
	}
}