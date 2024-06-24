#include "rom/MCB5.hpp"
#include <fstream>
#include <iostream>

uint8_t MCB5::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		// rom_banks 00-1FF (00 is treated as 01)
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable) {
			// printf("ram_bank: %u rom_ram_mode %u addr: %#06x\n", ram_bank, rom_ram_mode, addr);
			return ram_banks[ram_bank][addr - 0xA000];
		}
		return 0xFF; // TODO should return open bus values?
	default:
		std::cout << "should not reach this" << std::endl;
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
		}
        else if (addr <= 0x3FFF) {
            rom_bank = (ram_bank & 0xFF) | ((val & 1) << 8); //9th bit
        }
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x0F;
        }
		break;

	case 0xA000 ... 0xBFFF:
        if (ram_enable) {
            // printf("ram_bank: %u addr: %#06x val: %u\n", ram_bank, addr, val);
            ram_banks[ram_bank][addr - 0xA000] = val;
        }
		break;
	default:
		std::cout << "should not reach this" << std::endl;
		break;
	}
}

void MCB5::serialize(std::ofstream &f) {
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

void MCB5::deserialize(std::ifstream &f) {
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
