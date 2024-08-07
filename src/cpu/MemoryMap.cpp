#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "rom/MCB1.hpp"
#include "rom/MCB2.hpp"
#include "rom/MCB3.hpp"
#include "rom/MCB5.hpp"
#include "rom/RomHeader.hpp"
#include "rom/RomOnly.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>

MemoryMap::MemoryMap(const std::string path, Cpu *cpu) : cpu(cpu), header(path) {
	std::ifstream cgb("cgb_boot.bin", std::ios::binary | std::ios::ate);
	cgb.seekg(0, std::ios::beg);
	cgb.read(reinterpret_cast<char *>(&cgb_boot_rom), sizeof(cgb_boot_rom));
	cgb.close();

	// header.disable_cgb_enhancement();
	switch (header.cartridge_type()) {
	case CartridgeType::MBC1:
	case CartridgeType::MBC1_RAM:
	case CartridgeType::MBC1_RAM_BATTERY:
		rom = Rom::make<MCB1>(path, header, header.has_battery());
		break;
	case CartridgeType::MBC2:
	case CartridgeType::MBC2_BATTERY:
		rom = Rom::make<MCB2>(path, header, header.has_battery());
		break;
	case CartridgeType::MBC3_TIMER_BATTERY:
	case CartridgeType::MBC3_TIMER_RAM_BATTERY:
	case CartridgeType::MBC3:
	case CartridgeType::MBC3_RAM:
	case CartridgeType::MBC3_RAM_BATTERY:
		rom = Rom::make<MCB3>(path, header, header.has_battery());
		break;
	case CartridgeType::MBC5:
	case CartridgeType::MBC5_RAM:
	case CartridgeType::MBC5_RAM_BATTERY:
	case CartridgeType::MBC5_RUMBLE:
	case CartridgeType::MBC5_RUMBLE_RAM:
	case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
		rom = Rom::make<MCB5>(path, header, header.has_battery(), header.has_rumble());
		break;
	case CartridgeType::ROM_ONLY:
	case CartridgeType::ROM_RAM:
	case CartridgeType::ROM_RAM_BATTERY:
		rom = Rom::make<RomOnly>(path, header, header.has_battery());
		break;
	default:
		rom = Rom::make<MCB1>(path, header, header.has_battery());
		break;
	}

	// TODO handle MBC7, cartridge types and give an error when not mbc1,3,5,7 or Rom only roms
	// TODO handle (ROM_ONLY, ROM_RAM, ROM_RAM_BATTERY)
	// TODO add a way to force CGB_Enhanced roms in GB mode
}

MemoryMap::~MemoryMap() {}

// TODO check if this is even needed?
void MemoryMap::init_memory() {
	const uint8_t *initial_values;
	if (is_cgb_rom()) {
		initial_values = InitialIOValuesCGB;
	} else {
		initial_values = InitialIOValuesGB;
	}
	for (int i = 0; i < 65536; i++) {
		if ((i >= 0xC000) && (i < 0xE000)) {
			if ((i & 0x8) ^ ((i & 0x800) >> 8)) {
				if (is_cgb_rom()) {
					write_u8(i, 0x00);
					if (i >= 0xD000) {
						for (int a = 0; a < 8; a++) {
							if (a != 2) {
								work_ram[a][i & 0x0FFF] = read_u8(i - 0x1000);
								echo_ram[a][i & 0x0FFF] = read_u8(i - 0x1000);
							} else {
								work_ram[2][i & 0x0FFF] = 0x00;
								echo_ram[2][i & 0x0FFF] = 0x00;
							}
						}
					}
				} else {
					write_u8(i, 0x0F);
				}
			} else {
				write_u8(i, 0xFF);
				if (i >= 0xD000) {
					for (int a = 0; a < 8; a++) {
						if (a != 2) {
							work_ram[a][i & 0x0FFF] = read_u8(i - 0x1000);
							echo_ram[a][i & 0x0FFF] = read_u8(i - 0x1000);
						} else {
							work_ram[2][i & 0x0FFF] = 0x00;
							echo_ram[2][i & 0x0FFF] = 0x00;
						}
					}
				}
			}
		} else if (i >= 0xFF00) {
			switch (i) {
			case 0xFF0F:
				cpu->interrupt().overwrite_interrupt(initial_values[i - 0xFF00]);
				break;
			case 0xFF80 ... 0xFFFE:
				high_ram[i - 0xFF80] = initial_values[i - 0xFF00];
				break;
			case 0xFF40 ... 0xFF43:
			case 0xFF45 ... 0xFF4f:
			case 0xFF70:
				cpu->get_ppu().write_u8_ppu(i, initial_values[i - 0xFF00]);
				break;
			default:
				io_registers[i - 0xFF00] = initial_values[i - 0xFF00];
				break;
			}

		} else {
			write_u8(i, 0xFF);
		}
	}
}

uint8_t MemoryMap::read_u8(uint16_t addr) {
	// printf("trying to read addr: %#06x\n", addr);
	switch (addr) {
	case 0x0000 ... 0x7FFF:
		if (!boot_rom_loaded) {
			if (rom->cgb_mode()) {
				if (((addr < 0x0100) || (addr < 0x0900 && addr > 0x01FF))) {
					return cgb_boot_rom[addr];
				}
			} else if (addr <= 0xFF) {
				return gb_boot_rom[addr];
			}
		}
		return rom->read_u8(addr);
	case 0x8000 ... 0x9FFF:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xA000 ... 0xBFFF:
		return rom->read_u8(addr);
	case 0xC000 ... 0xDFFF:
		if (addr <= 0xCFFF) {
			return work_ram[0][uint16_t(addr & 0x0FFF)];
		} else {
			if (is_cgb_rom()) {
				return work_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x1000];
			} else {
				return work_ram[1][uint16_t(addr & 0x1FFF) - 0x1000];
			}
		}
	case 0xE000 ... 0xFDFF:
		if (addr <= 0xEFFF) {
			return echo_ram[0][uint16_t(addr & 0x0FFF)];
		} else {
			if (is_cgb_rom()) {
				return echo_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x1000];
			} else {
				return echo_ram[1][uint16_t(addr & 0x0FFF) - 0x1000];
			}
		}
	case 0xFE00 ... 0xFE9F:
		return cpu->get_ppu().read_oam(addr);
	case 0xFEA0 ... 0xFEFF:
		return not_usable[addr - 0xFEA0];
	case 0xFF00 ... 0xFF3F:
		switch (addr) {
		case 0xFF00:
			switch (joypad) {
			case 1:
				return joypad_buttons;
			case 2:
				return joypad_dpad;
			default:
				return 0xFF;
			}
		case 0xFF04:
			return cpu->interrupt().get_timer_divider();
		case 0xFF05:
			return cpu->interrupt().get_timer_counter();
		case 0xFF06:
			return cpu->interrupt().get_timer_module();
		case 0xFF07:
			return cpu->interrupt().get_timer_control();
		case 0xFF0F:
			return cpu->interrupt().get_interrupt() | 0xE0;
		default:
			return io_registers[addr - 0xFF00];
		}
	case 0xFF40 ... 0xFF4F:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xFF51 ... 0xFF7F:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xFF50:
		return io_registers[(std::size_t)(0xFF50 - 0xFF00)];
	case 0xFF80 ... 0xFFFE:
		return high_ram[addr - 0xFF80];
	case 0xFFFF:
		return cpu->interrupt().get_interrupt_enable();

	default:
		return 0;
	}
}

uint16_t MemoryMap::read_u16(uint16_t addr) {
	return ((uint16_t)read_u8(addr) + ((uint16_t)read_u8(addr + 1) << 8));
}

void MemoryMap::write_u8(uint16_t addr, uint8_t val) {
	// printf("trying to write to addr: %#06x with val: %u\n", addr, val);
	switch (addr) {
	case 0x0000 ... 0x7FFF:
		if (boot_rom_loaded) {
			rom->write_u8(addr, val);
		}
		break;
	case 0x8000 ... 0x9FFF:
		cpu->get_ppu().write_u8_ppu(addr, val);
		break;
	case 0xA000 ... 0xBFFF:
		rom->write_u8(addr, val);
		break;
	case 0xC000 ... 0xDFFF:
		if (addr <= 0xCFFF) {
			work_ram[0][uint16_t(addr & 0x0FFF)] = val;
		} else {
			if (is_cgb_rom()) {
				work_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x1000] = val;
			} else {
				work_ram[1][uint16_t(addr & 0x0FFF)] = val;
			}
		}
		break;
	case 0xE000 ... 0xFDFF:
		if (addr <= 0xEFFF) {
			echo_ram[0][uint16_t(addr & 0x0FFF)] = val;
		} else {
			if (is_cgb_rom()) {
				echo_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x1000] = val;
			} else {
				echo_ram[1][uint16_t(addr & 0x0FFF)] = val;
			}
		}
		break;
	case 0xFE00 ... 0xFE9F:
		cpu->get_ppu().write_oam(addr, val);
		break;
	case 0xFEA0 ... 0xFEFF:
		not_usable[addr - 0xFEA0] = val;
		break;
	case 0xFF00 ... 0xFF3F:
		switch (addr) {
		case 0xFF00:
			joypad = (val >> 4) & 3;
			break;
		case 0xFF04:
			cpu->interrupt().reset_timer_divider();
			break;
		case 0xFF05:
			cpu->interrupt().set_timer_counter(val);
			break;
		case 0xFF06:
			cpu->interrupt().set_timer_modulo(val);
			break;
		case 0xFF07:
			cpu->interrupt().set_timer_control(val); // TODO slightly more complex
			break;
		case 0xFF0F:
#ifdef DEBUG_MODE
			printf("changing interrupt: %u\n", val & 0x1F);
#endif
			cpu->interrupt().overwrite_interrupt(val & 0x1F);
			// cpu->overwrite_interrupt(val); // TODO check
			break;
		default:
#ifdef DEBUG_MODE
			if (addr == 0xFF26) {
				printf("writing to audio channel at 0xFF26: %u\n", val);
			}
#endif
			io_registers[addr - 0xFF00] = val;
			break;
		}
		break;
	case 0xFF40 ... 0xFF4F:
		cpu->get_ppu().write_u8_ppu(addr, val);
		break;
	case 0xFF51 ... 0xFF7F:
		cpu->get_ppu().write_u8_ppu(addr, val);
		break;
	case 0xFF50:
		if (!boot_rom_loaded && (val & 0x01) > 0) {
			boot_rom_loaded = true;
#ifdef DEBUG_MODE
			printf("boot_rom loaded\n");
#endif
		}
		// io_registers[(std::size_t)(0xFF50 - 0xFF00)] = val;
		break;
	case 0xFF80 ... 0xFFFE:
		high_ram[addr - 0xFF80] = val;
		break;
	case 0xFFFF:
		cpu->interrupt().set_interrupt_enable(val);
		break;

	default:
		break;
	}
}

void MemoryMap::write_u16(uint16_t addr, uint16_t val) {
	write_u8(addr, (uint8_t)(val & 0xFF));
	write_u8(addr + 1, (uint8_t)((val & 0xFF00) >> 8));
}

uint8_t MemoryMap::wram_bank_select() {
	return cpu->get_ppu().get_wram_bank_select();
}

uint8_t MemoryMap::read_io_registers(uint16_t addr) {
	return io_registers[addr - 0xFF00];
}

void MemoryMap::write_io_registers(uint16_t addr, uint8_t val) {
	io_registers[addr - 0xFF00] = val;
}
