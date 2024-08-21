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
#include <cstdlib>
#include <fstream>
#include <iostream>

MemoryMap::MemoryMap(const options options, Cpu *cpu) : header(options.path), cpu(cpu) {
	switch (header.cartridge_type()) {
	case CartridgeType::MBC1 ... MBC1_RAM_BATTERY:
		rom = Rom::make<MCB1>(options.path, header, header.has_battery());
		break;
	case CartridgeType::MBC2 ... MBC2_BATTERY:
		rom = Rom::make<MCB2>(options.path, header, header.has_battery());
		break;
	case CartridgeType::MBC3_TIMER_BATTERY ... MBC3_RAM_BATTERY:
		rom = Rom::make<MCB3>(options.path, header, header.has_battery());
		break;
	case CartridgeType::MBC5 ... MBC5_RUMBLE_RAM_BATTERY:
		rom = Rom::make<MCB5>(options.path, header, header.has_battery(), header.has_rumble());
		break;
	case CartridgeType::ROM_ONLY ... ROM_RAM_BATTERY:
		rom = Rom::make<RomOnly>(options.path, header, header.has_battery());
		break;
	default:
		std::cerr << "Cartridge type: " << header.cartridge_type() << " not supported" << std::endl;
		cpu->close();
		exit(EXIT_FAILURE);
		break;
	}

	if ((is_cgb_rom() && !options.force_dmg) || options.force_cgb) {
		std::ifstream cgb("cgb_boot.bin", std::ios::binary | std::ios::ate);
		if (!cgb.is_open()) {
			std::cerr << "Error: Failed to  open file cgb boot rom." << std::endl;
			exit(EXIT_FAILURE);
		}
		cgb.seekg(0, std::ios::beg);
		cgb.read(reinterpret_cast<char *>(&cgb_boot_rom), sizeof(cgb_boot_rom));
		cgb.close();
		if (options.force_cgb && !is_cgb_rom()) {
			header.force_cgb_enhancement();
			rom->force_cgb_mode();
		}
	}
	else {
		std::ifstream cgb("dmg_boot.bin", std::ios::binary | std::ios::ate);
		if (!cgb.is_open()) {
			std::cerr << "Error: Failed to  open file dmg boot rom." << std::endl;
			exit(EXIT_FAILURE);
		}
		cgb.seekg(0, std::ios::beg);
		cgb.read(reinterpret_cast<char *>(&gb_boot_rom), sizeof(gb_boot_rom));
		cgb.close();
		header.disable_cgb_enhancement();
		rom->force_dmg_mode();
	}

	// check_color_rgb555(0xff, 0x15, 0x1c);
	// check_color_rgb555(0xe0, 0x1b, 0x24);
	// check_color_rgb555(0xa5, 0x1d, 0x2d);
	// check_color_rgb555(0x1A, 0x1A, 0x1A);
}

MemoryMap::~MemoryMap() {}

void MemoryMap::init_memory() {
	const uint8_t *initial_values;
	if (is_cgb_rom()) {
		initial_values = initial_io_values_dmg;
	} else {
		initial_values = initial_io_values_dmg;
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
			default:
				io_registers[i - 0xFF00] = initial_values[i - 0xFF00];
				break;
			}

		} else if (i < 0XA000 || i > 0xBFFF) {
			write_u8(i, 0xFF);
		}
	}
}

uint8_t MemoryMap::read_u8(uint16_t addr) {
	// DEBUG_MSG("trying to read addr: %#06x\n", addr);
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
			return joypad_register;
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
		case 0xFF02:
			return cpu->interrupt().get_serial_transfer_control();
		case 0xFF01:
			return cpu->interrupt().get_serial_transfer_data();
		case 0xFF10 ... 0xFF3F:
			// return io_registers[addr - 0xFF00];
			return cpu->get_apu().read_u8(addr);
		default:
			return io_registers[addr - 0xFF00];
		}
	case 0xFF40 ... 0xFF4F:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xFF51 ... 0xFF7F:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xFF50:
		return io_registers[0x50];
	case 0xFF80 ... 0xFFFE:
		return high_ram[addr - 0xFF80];
	case 0xFFFF:
		return cpu->interrupt().get_interrupt_enable();

	default:
		return 0;
	}
}

uint16_t MemoryMap::read_u16(uint16_t addr) {
	return (static_cast<uint16_t>(read_u8(addr)) + static_cast<uint16_t>(read_u8(addr + 1) << 8));
}

void MemoryMap::write_u8(uint16_t addr, uint8_t val) {
	// DEBUG_MSG("trying to write to addr: %#06x with val: %u\n", addr, val);
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
			work_ram[0][(addr & 0x0FFF)] = val;
		} else {
			if (is_cgb_rom()) {
				work_ram[wram_bank_select()][(addr & 0x1FFF) - 0x1000] = val;
			} else {
				work_ram[1][(addr & 0x0FFF)] = val;
			}
		}
		break;
	case 0xE000 ... 0xFDFF:
		if (addr <= 0xEFFF) {
			echo_ram[0][(addr & 0x0FFF)] = val;
		} else {
			if (is_cgb_rom()) {
				echo_ram[wram_bank_select()][(addr & 0x1FFF) - 0x1000] = val;
			} else {
				echo_ram[1][(addr & 0x0FFF)] = val;
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
			DEBUG_MSG("writing to 0xFF00: %u\t", val);
			joypad_register = (joypad_register & 0xCF) | (val & 0x30);
			update_joypad();
			DEBUG_MSG("after writing to 0xFF00: %u\n", read_u8(0xFF00));
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
			cpu->interrupt().set_timer_control(val);
			break;
		case 0xFF02:
			cpu->interrupt().set_serial_transfer_control(val);
			break;
		case 0xFF01:
			cpu->interrupt().set_serial_transfer_data(val);
			break;
		case 0xFF0F:
			DEBUG_MSG("changing interrupt write: %u\n", val & 0x1F);
			cpu->interrupt().overwrite_interrupt(val & 0x1F);
			break;
		case 0xFF10 ... 0xFF3F:
			cpu->get_apu().write_u8(addr, val);
			io_registers[addr - 0xFF00] = val;
		break;
		default:
#ifdef DEBUG_MODE
			if (addr == 0xFF26) {
				DEBUG_MSG("writing to audio channel at 0xFF26: %u\n", val);
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
			DEBUG_MSG("boot_rom loaded\n");
		}
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
	write_u8(addr, static_cast<uint8_t>(val & 0xFF));
	write_u8(addr + 1, static_cast<uint8_t>((val & 0xFF00) >> 8));
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

void MemoryMap::update_joypad() {
	uint8_t current = joypad_register & 0xF0;

	switch (current & 0x30) {
	case 0x10: {
		uint8_t top = (joypad_pressed >> 4) & 0x0F;
		current |= top;
		break;
	}
	case 0x20: {
		uint8_t bottom = joypad_pressed & 0x0F;
		current |= bottom;
		break;
	}
	case 0x30:
		current |= 0x0F;
		break;
	}

	if ((joypad_register & ~current & 0x0F) != 0) {
		cpu->interrupt().set_interrupt(interrupt_type::Joypad);
	}

	joypad_register = current;
}

void MemoryMap::save_ram() {
	rom->save_ram();
}

void MemoryMap::load_ram() {
	// TODO maybe have it optionally load the ram?
	rom->load_ram();
}
