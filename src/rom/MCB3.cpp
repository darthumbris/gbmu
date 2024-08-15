#include "rom/MCB3.hpp"
#include "debug.hpp"
#include <fstream>
#include <iostream>

uint8_t MCB3::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		return ram_banks[ram_bank][addr - 0xA000];
	default:
		std::cout << "should not reach this" << std::endl;
		return 0xFF;
	}
}

void MCB3::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x1FFF) {
			ram_enable = (val == 0x0A);
		} else if (addr <= 0x3FFF) {
			rom_bank = val ? val : 1;
		} else if (addr <= 0x5FFF) {
			if (val < 0x04) {
				ram_bank = val;
			} else if (val >= 0x08 && val <= 0x0C) {
				// TODO should map the corresponding RTC register to A000 - BFFF ?
			}
		} else if (addr <= 0x7FFF) {
			// TODO latched stuff (when writing 0x00 then 0x01 to this, current time becomes latched to the RTC
			// register) the latched data will not change until it becomes latched again by repeating the 0x00 -> 0x01
			// procedure
		}
		break;

	case 0xA000 ... 0xBFFF:
		if (ram_enable) {
			DEBUG_MSG("ram_bank: %u addr: %#06x val: %u\n", ram_bank, addr, val);
			ram_banks[ram_bank][addr - 0xA000] = val;
		}
		break;
	default:
		std::cout << "should not reach this" << std::endl;
		break;
	}
}

void MCB3::serialize(std::ofstream &f) {
	for (int i = 0; i < rom_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < ram_banks.size(); i++) {
		f.write(reinterpret_cast<const char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.write(reinterpret_cast<const char *>(&rom_bank), sizeof(rom_bank));
	f.write(reinterpret_cast<const char *>(&ram_bank), sizeof(ram_bank));
	f.write(reinterpret_cast<const char *>(&ram_enable), sizeof(ram_enable));
	f.write(reinterpret_cast<const char *>(&seconds), sizeof(seconds));
	f.write(reinterpret_cast<const char *>(&minutes), sizeof(minutes));
	f.write(reinterpret_cast<const char *>(&hours), sizeof(hours));
	f.write(reinterpret_cast<const char *>(&day_l), sizeof(day_l));
	f.write(reinterpret_cast<const char *>(&day_h), sizeof(day_h));
	f.write(reinterpret_cast<const char *>(&latched), sizeof(latched));
	std::cout << "done serializing rom" << std::endl;
}

void MCB3::deserialize(std::ifstream &f) {
	for (int i = 0; i < rom_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.read(reinterpret_cast<char *>(&rom_bank), sizeof(rom_bank));
	f.read(reinterpret_cast<char *>(&ram_bank), sizeof(ram_bank));
	f.read(reinterpret_cast<char *>(&ram_enable), sizeof(ram_enable));
	f.read(reinterpret_cast<char *>(&seconds), sizeof(seconds));
	f.read(reinterpret_cast<char *>(&minutes), sizeof(minutes));
	f.read(reinterpret_cast<char *>(&hours), sizeof(hours));
	f.read(reinterpret_cast<char *>(&day_l), sizeof(day_l));
	f.read(reinterpret_cast<char *>(&day_h), sizeof(day_h));
	f.read(reinterpret_cast<char *>(&latched), sizeof(latched));
	std::cout << "done deserializing rom" << std::endl;
}

void MCB3::save_ram() {
	if (battery) {
		Rom::save_ram();
	}
}

void MCB3::load_ram() {
	if (battery) {
		Rom::load_ram();
	}
}