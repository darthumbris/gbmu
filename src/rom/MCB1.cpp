#include "rom/MCB1.hpp"
#include <fstream>
#include <iostream>

uint8_t MCB1::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		// TODO if rom > 1Mib this can also be different rom banks
		if (rom_ram_mode) {
			printf("rom_bank + secondary: %u\n", rom_bank + secondary_rom_bank);
			return rom_banks[0][addr];
		}
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		// rom_banks 01-7F (00 is treated as 01)
		if (!rom_bank) {
			printf("rom_bank: %u rom_ram_mode %u addr: %#06x\n", rom_bank == 0 ? 1 : rom_bank, rom_ram_mode, addr);
		}
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable) {
			// printf("ram_bank: %u rom_ram_mode %u addr: %#06x\n", ram_bank, rom_ram_mode, addr);
			return ram_banks[ram_bank][addr - 0xA000]; // TODO make sure ext_ram and ram_bank are correct()
			                                           // return ram_banks[0][addr - 0xA000];
		}
		// return ram_banks[ram_bank][addr - 0xA000];
		return 0xFF; // TODO should return open bus values?
	default:
		std::cout << "should not reach this" << std::endl;
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
			printf("rom_ram_mode: %u\n", rom_ram_mode);
		}
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x03 * rom_ram_mode;
			secondary_rom_bank = (val & 0x04 * rom_ram_mode) << 6;
		}
		break;

	case 0xA000 ... 0xBFFF:
		// ram_banks[0][addr - 0xA000] = val;
		if (ram_enable) {
			// printf("ram_bank: %u addr: %#06x val: %u\n", ram_bank, addr, val);
			ram_banks[ram_bank][addr - 0xA000] = val; // TODO fix this
		}
		break;
	default:
		std::cout << "should not reach this" << std::endl;
		break;
	}
}

void MCB1::serialize(std::ofstream &f) {
	for (int i = 0; i < rom_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < ram_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.write(reinterpret_cast<const char *>(&rom_bank), sizeof(rom_bank));
	f.write(reinterpret_cast<const char *>(&ram_bank), sizeof(ram_bank));
	f.write(reinterpret_cast<const char *>(&ram_enable), sizeof(ram_enable));
	std::cout << "done serializing rom" << std::endl;
}

void MCB1::deserialize(std::ifstream &f) {
	for (int i = 0; i < rom_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.read(reinterpret_cast<char *>(&rom_bank), sizeof(rom_bank));
	f.read(reinterpret_cast<char *>(&ram_bank), sizeof(ram_bank));
	f.read(reinterpret_cast<char *>(&ram_enable), sizeof(ram_enable));
	std::cout << "done deserializing rom" << std::endl;
}
