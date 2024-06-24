#include "MCB3.hpp"

uint8_t MCB3::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		// printf("ram_bank: %u rom_ram_mode %u addr: %#06x\n", ram_bank, rom_ram_mode, addr);
        return ram_banks[ram_bank][addr - 0xA000]; //TODO make sure ext_ram and ram_bank are correct()
		// return ram_banks[0][addr - 0xA000];
	default:
		std::cout << "should not reach this" << std::endl;
		return 0xFF;
	}
}

uint16_t MCB3::read_u16(uint16_t addr) {
	return ((uint16_t)read_u8(addr) + ((uint16_t)read_u8(addr + 1) << 8));
}

void MCB3::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x000 ... 0x7FFF:
		if (addr <= 0x1FFF) {
			if (val == 0x0A) {
				ram_enable = true;
            }
			else {
				ram_enable = false;
            }
		} else if (addr <= 0x2FFF) {
			rom_bank = (ram_bank & (1 << 8)) | val;
		} else if (addr <= 0x3FFF) {
			rom_bank = (ram_bank & 0xFF) | ((val & 1) << 8);
		}
        if (addr >= 0x6000 && addr <= 0x7FFF && (val == 0x00 || val == 0x01)) {
			rom_ram_mode = val == 0x01;
        }
		if (addr >= 0x4000 && addr <= 0x5FFF) {
			ram_bank = val & 0x0F * rom_ram_mode;
        }
		break;

	case 0xA000 ... 0xBFFF:
		// ram_banks[0][addr - 0xA000] = val;
        if (ram_enable) {
            printf("ram_bank: %u addr: %#06x val: %u\n", ram_bank, addr, val);
            ram_banks[ram_bank][addr - 0xA000] = val; //TODO fix this
        }
		break;
	default:
		std::cout << "should not reach this" << std::endl;
		break;
	}
}

void MCB3::write_u16(uint16_t addr, uint16_t val) {
	write_u8(addr, (uint8_t)(val & 0xFF));
	write_u8(addr + 1, (uint8_t)((val & 0xFF00) >> 8));
}