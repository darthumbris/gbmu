#include "MemoryMap.hpp"
#include "Cpu.hpp"
#include "rom/MCB1.hpp"
#include "rom/MCB2.hpp"
#include "rom/MCB3.hpp"
#include "rom/MCB5.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>

MemoryMap::MemoryMap(const std::string path, Cpu *cpu) : cpu(cpu), header(path) {
	std::ifstream cgb("cgb_boot.bin", std::ios::binary | std::ios::ate);
	std::cout << "cgb_boot size: " << cgb.tellg() << std::endl;
	cgb.seekg(0, std::ios::beg);
	cgb.read(reinterpret_cast<char *>(&cgb_boot_rom), sizeof(cgb_boot_rom));
	cgb.close();

    switch (header.cartridge_type())
    {
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
        rom = Rom::make<MCB1>(path, header, header.has_battery());
        break;
    case CartridgeType::MBC5:
    case CartridgeType::MBC5_RAM:
    case CartridgeType::MBC5_RAM_BATTERY:
    case CartridgeType::MBC5_RUMBLE:
    case CartridgeType::MBC5_RUMBLE_RAM:
    case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
        rom = Rom::make<MCB5>(path, header, header.has_battery(), header.has_rumble());
        break;
    default:
        rom = Rom::make<MCB1>(path, header, header.has_battery());
        break;
    }
	// TODO handle different cartridge types
}

MemoryMap::~MemoryMap() {}

INLINE_FN uint8_t MemoryMap::read_u8(uint16_t addr) {
	// printf("trying to read addr: %#06x\n", addr);
	switch (addr) {
	case 0x0000 ... 0x7FFF:
		if (!boot_rom_loaded) {
			if (rom->cgb_mode()) {
				return cgb_boot_rom[addr];
			} else if (addr <= 0xFF) {
				return gb_boot_rom[addr];
			}
		}
		return rom->read_u8(addr);
		;
	case 0x8000 ... 0x9FFF:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xA000 ... 0xBFFF:
		return rom->read_u8(addr);
	case 0xC000 ... 0xDFFF:
		if (addr <= 0xCFFF) {
			return work_ram[0][uint16_t(addr & 0x0FFF)];
		} else {
			return work_ram[1][uint16_t(addr & 0x0FFF)];
			// fix this return work_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF];
		}
	case 0xE000 ... 0xFDFF:
		if (addr <= 0xEFFF) {
			return echo_ram[0][uint16_t(addr & 0x0FFF)];
		} else {
			return echo_ram[1][uint16_t(addr & 0x0FFF)];
			// TODO fix this return echo_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF];
		}
	case 0xFE00 ... 0xFE9F:
		return cpu->get_ppu().read_oam(addr & 0xFF);
	case 0xFEA0 ... 0xFEFF:
		return not_usable[addr - 0xFEA0];
	case 0xFF00 ... 0xFF3F:
		if (addr == 0xFF00) {
			switch (joypad) {
			case 1:
				return joypad_buttons;
			case 2:
				return joypad_dpad;
			default:
				return 0xFF;
			}
		}
		if (addr == 0xFF04) {
			return cpu->get_timer_divider();
		}
		return io_registers[addr - 0xFF00];
	case 0xFF40 ... 0xFF4F:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xFF51 ... 0xFF7F:
		return cpu->get_ppu().read_u8_ppu(addr);
	case 0xFF50:
		return io_registers[(std::size_t)(0xFF50 - 0xFF00)];
	case 0xFF80 ... 0xFFFE:
		return high_ram[addr - 0xFF80];
	case 0xFFFF:
		return cpu->get_interrupt_enable();

	default:
		return 0;
	}
}

INLINE_FN uint16_t MemoryMap::read_u16(uint16_t addr) {
	return ((uint16_t)read_u8(addr) + ((uint16_t)read_u8(addr + 1) << 8));
}

INLINE_FN void MemoryMap::write_u8(uint16_t addr, uint8_t val) {
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
			work_ram[1][uint16_t(addr & 0x0FFF)] = val;
			// TODO fix this work_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF] = val;
		}
		break;
	case 0xE000 ... 0xFDFF:
		if (addr <= 0xEFFF) {
			echo_ram[0][uint16_t(addr & 0x0FFF)] = val;
		} else {
			echo_ram[1][uint16_t(addr & 0x0FFF)] = val;
			// TODO fix this echo_ram[wram_bank_select()][uint16_t(addr & 0x1FFF) - 0x0FFF] = val;
		}
		break;
	case 0xFE00 ... 0xFE9F:
		cpu->get_ppu().write_oam(addr & 0xFF, val);
		break;
	case 0xFEA0 ... 0xFEFF:
		not_usable[addr - 0xFEA0] = val;
		break;
	case 0xFF00 ... 0xFF3F:
		if (addr == 0xFF00) {
			joypad = (val >> 4) & 3;
			break;
		}
		if (addr == 0xFF0F) {
			cpu->overwrite_interrupt(val);
			break;
		}
		if (addr == 0xFF04) {
			cpu->reset_timer_divider();
			break;
		}
		io_registers[addr - 0xFF00] = val;
		break;
	case 0xFF40 ... 0xFF4F:
		cpu->get_ppu().write_u8_ppu(addr, val);
		break;
	case 0xFF51 ... 0xFF7F:
		cpu->get_ppu().write_u8_ppu(addr, val);
		break;
	case 0xFF50:
		if (!boot_rom_loaded) {
			boot_rom_loaded = true;
		}
		io_registers[(std::size_t)(0xFF50 - 0xFF00)] = val;
		break;
	case 0xFF80 ... 0xFFFE:
		high_ram[addr - 0xFF80] = val;
		break;
	case 0xFFFF:
		cpu->set_interrupt_enable(val);
		break;

	default:
		break;
	}
}

INLINE_FN void MemoryMap::write_u16(uint16_t addr, uint16_t val) {
	write_u8(addr, (uint8_t)(val & 0xFF));
	write_u8(addr + 1, (uint8_t)((val & 0xFF00) >> 8));
}

uint8_t MemoryMap::wram_bank_select() {
	return cpu->get_ppu().read_u8_ppu(0xFF70);
}
