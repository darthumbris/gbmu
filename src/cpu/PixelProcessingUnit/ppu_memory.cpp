#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"
#include <cstdint>

void PixelProcessingUnit::init_ppu_mem() {
	const uint8_t *initial_values;
	if (is_cgb) {
		initial_values = InitialIOValuesCGB;
	} else {
		initial_values = InitialIOValuesGB;
	}
	for (int i = 0xFF00; i < 65536; i++) {
		switch (i) {
		case 0xFF40:
		case 0xFF42 ... 0xFF43:
		case 0xFF45 ... 0xFF4f:
		case 0xFF70:
			cpu->get_ppu().write_u8_ppu(i, initial_values[i - 0xFF00]);
			break;
		case 0xFF41:
			l_status.set(initial_values[i - 0xFF00]);
			l_status.mode = PPU_Modes::Vertical_Blank;
			break;
		default:
			break;
		}
	}
}

void PixelProcessingUnit::set_tile_data(uint16_t addr) {
	addr &= 0x1FFE;

	int tile_index = (addr >> 4) & 0x1FF;
	int y = (addr >> 1) & 7;
	if (tile_index == 384)
		return;

	for (int x = 0; x < 8; x++) {
		unsigned char bitmask = 1 << (7 - x);
		tile_data[vbank_select][tile_index][y * 8 + x] = static_cast<uint8_t>(
		    ((vram[vbank_select][addr] & bitmask) ? 1 : 0) + ((vram[vbank_select][addr + 1] & bitmask) ? 2 : 0));
	}
}

void PixelProcessingUnit::write_oam(uint16_t addr, uint8_t val) {
	uint16_t sprite_addr = addr & 0xFF;
	oam[sprite_addr / 4][sprite_addr % 4] = val;
	switch (sprite_addr & 3) {
	case 0:
		sprites[sprite_addr >> 2].y_pos = val;
		break;
	case 1:
		sprites[sprite_addr >> 2].x_pos = val;
		break;
	case 2:
		sprites[sprite_addr >> 2].tile_index = val;
		break;
	case 3:
		sprites[sprite_addr >> 2].attributes.set(val);
		break;
	}
}

uint8_t PixelProcessingUnit::read_oam(uint16_t addr) {
	uint16_t sprite_addr = addr & 0xFF;
	return oam[sprite_addr / 4][sprite_addr % 4];
}

Sprite PixelProcessingUnit::read_sprite(uint16_t addr) {
	uint16_t sprite_addr = addr & 0xFF;
	return sprites[sprite_addr >> 2];
}

uint8_t PixelProcessingUnit::read_cgb_vram(uint16_t addr, bool force) {
	if (force || (vbank_select == 1)) {
		return vram[1][addr - 0x8000];
	} else {
		return vram[0][addr - 0x8000];
	}
}

uint8_t PixelProcessingUnit::read_u8_ppu(uint16_t addr) {
	switch (addr) {
	case 0x8000 ... 0x9FFF:
		return vram[vbank_select][addr & 0x1FFF];
	case 0xFF40:
		return ctrl.val;
	case 0xFF41:
		return l_status.get() | 0x80;
	case 0xFF42:
		return scy;
	case 0xFF43:
		return scx;
	case 0xFF44:
		return lcd_enabled ? ly : 0x00;
	case 0xFF45:
		return lyc;
	case 0xFF46:
		return dma.val;
	case 0xFF47:
		return bg_palette;
	case 0xFF48:
		return obj_palette_0;
	case 0xFF49:
		return obj_palette_1;
	case 0xFF4A:
		return window_y;
	case 0xFF4B:
		return window_x;
	case 0xFF4C:
		return 0xFF;
	case 0xFF4F:
		return cpu->get_mmap().read_io_registers(addr) | 0xFE;
	case 0xFF51:
		return is_cgb ? get_hdma_register(1) : cpu->get_mmap().read_io_registers(addr);
	case 0xFF52:
		return is_cgb ? get_hdma_register(2) : cpu->get_mmap().read_io_registers(addr);
	case 0xFF53:
		return is_cgb ? get_hdma_register(3) : cpu->get_mmap().read_io_registers(addr);
	case 0xFF54:
		return is_cgb ? get_hdma_register(4) : cpu->get_mmap().read_io_registers(addr);
	case 0xFF55:
		return is_cgb ? get_hdma_register(5) : cpu->get_mmap().read_io_registers(addr);
	case 0xFF68:
		return is_cgb ? (cpu->get_mmap().read_io_registers(addr) | 0x40) : 0xC0;
	case 0xFF69:
		return is_cgb ? (cpu->get_mmap().read_io_registers(addr)) : 0xFF;
	case 0xFF6A:
		return is_cgb ? (cpu->get_mmap().read_io_registers(addr) | 0x40) : 0xC0;
	case 0xFF6B:
		return is_cgb ? (cpu->get_mmap().read_io_registers(addr)) : 0xFF;
	case 0xFF70:
		return is_cgb ? (cpu->get_mmap().read_io_registers(addr) | 0xF8) : 0xFF;
	case 0xFF76:
	case 0xFF77:
		return is_cgb ? 0x00 : 0xFF;
	default:
		return cpu->get_mmap().read_io_registers(addr);
	}
}

void PixelProcessingUnit::write_u8_ppu(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x8000 ... 0x9FFF:
		vram[vbank_select][addr & 0x1FFF] = val;
		if (addr < 0x9800) {
			set_tile_data(addr);
		}
		break;
	case 0xFF40: {
		bool old_window_enable = ctrl.window_enable;
		ctrl.set(val);
		if (!old_window_enable && ctrl.window_enable) {
			if ((window_line_active == 0) && (ly < 144) && (ly > window_y)) {
				window_line_active = 144;
			}
		}
		if (ctrl.lcd_enable) {
			enable_screen();
		} else {
			disable_screen();
		}
		break;
	}
	case 0xFF41: {
		uint8_t current_mode = l_status.mode;
		uint8_t current_stat = l_status.val & 0x07;
		uint8_t new_stat = (val & 0x78) | (current_stat & 0x07);
		uint8_t signal = interrupt_signal;
		signal &= ((new_stat >> 3) & 0x0F);
		interrupt_signal = signal;
		l_status.set(new_stat);
		DEBUG_MSG("changing lcd stat write: %u\n", l_status.get());
		if (ctrl.lcd_enable) {
			if (l_status.mode_0_hblank_interrupt && current_mode == 0) {
				if (signal == 0) {
					cpu->interrupt().set_interrupt(InterruptType::Stat);
				}
				signal |= mask0;
			}
			if ((new_stat & mask4) && (current_mode == 1)) {
				if (signal == 0) {
					cpu->interrupt().set_interrupt(InterruptType::Stat);
				}
				signal |= mask1;
			}
			if ((new_stat & mask5) && (current_mode == 2)) {
				if (signal == 0) {
					cpu->interrupt().set_interrupt(InterruptType::Stat);
				}
			}
			compare_ly();
		}
		break;
	}
	case 0xFF42:
		scy = val;
		break;
	case 0xFF43:
		scx = val;
		break;
	case 0xFF44:
		if ((ly & mask7) && !(val & mask7)) {
			disable_screen();
		}
		break;
	case 0xFF45:
		if (lyc != val) {
			lyc = val;
			if (ctrl.lcd_enable) {
				compare_ly();
			}
		}
		break;
	case 0xFF46:
		dma.set(val);
		dma_transfer(val);
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF47:
		bg_palette = val;
		break;
	case 0xFF48:
		obj_palette_0 = val;
		break;
	case 0xFF49:
		obj_palette_1 = val;
		break;
	case 0xFF4A:
		window_y = val;
		break;
	case 0xFF4B:
		window_x = val;
		break;
	case 0xFF4F:
		if (is_cgb) {
			vbank_select = val & 0x01;
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF51:
		if (is_cgb) {
			set_hdma_register(HDMA_1, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF52:
		if (is_cgb) {
			set_hdma_register(HDMA_2, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF53:
		if (is_cgb) {
			set_hdma_register(HDMA_3, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF54:
		if (is_cgb) {
			set_hdma_register(HDMA_4, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF55:
		if (is_cgb) {
			switch_cgb_dma(val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF68:
		if (is_cgb) {
			bg_palette_cgb = val;
			update_palette_cgb(true, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF69:
		if (is_cgb) {
			set_color_palette(true, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF6A:
		if (is_cgb) {
			obj_palette_cgb = val;
			update_palette_cgb(false, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF6B:
		if (is_cgb) {
			obj_color_cgb = val;
			set_color_palette(false, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF6C:
		cpu->get_mmap().write_io_registers(addr, val | 0xFE);
		break;
	case 0xFF70:
		if (is_cgb) {
			wram_bank_select = val & 0x07;
			if (wram_bank_select == 0) {
				wram_bank_select = 1;
			}
		} else {
			wram_bank_select = 0;
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF75:
		cpu->get_mmap().write_io_registers(addr, val | 0x8F);
		break;
	default:
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	}
}
