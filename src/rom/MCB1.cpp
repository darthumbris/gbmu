#include "rom/MCB1.hpp"

uint8_t MCB1::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		if (rom_ram_mode) {
			return rom_banks[0][addr];
		}
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			return ram_banks[ram_bank][addr - 0xA000];
		}
		return 0xFF;
	default:
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
			rom_bank &= (uint8_t)rom_banks.size() - 1;
		}
		if (addr >= 0x6000 && addr <= 0x7FFF && (val == 0x00 || val == 0x01)) {
			rom_ram_mode = val == 0x01;
		}
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x03 * rom_ram_mode;
			ram_bank &= (uint8_t)ram_banks.size() - 1;
			secondary_rom_bank = (val & 0x04 * rom_ram_mode) << 6;
		}
		break;
	case 0xA000 ... 0xBFFF:
		if (ram_enable && ram_banks.size()) {
			ram_banks[ram_bank][addr - 0xA000] = val;
		}
		break;
	default:
		break;
	}
}

void MCB1::reset() {
	save_ram();
	rom_bank = 1;
	secondary_rom_bank = 0;
	ram_bank = 0;
	ram_enable = false;
	rom_ram_mode = false;
	load_ram();
}
