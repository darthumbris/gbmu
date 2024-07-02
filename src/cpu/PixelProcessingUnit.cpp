#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"
#include "MemoryMap.hpp"
#include <cstring>
#include <iostream>

PixelProcessingUnit::PixelProcessingUnit(Cpu *cpu) : cpu(cpu) {
	lcd_clock = 0;
	init_window();
	data.status = false;
	ctrl = {0};
	l_status = {0};
	l_status.mode = PPU_Modes::Pixel_Drawing;
	sprites = {0};
	std::memset(framebuffer, 0, sizeof(framebuffer));
	std::memset(vram, 0, sizeof(vram));
	std::memset(obj_0_colors, 0, sizeof(obj_0_colors));
	std::memset(obj_1_colors, 0, sizeof(obj_1_colors));
	std::memset(tile_data, 0, sizeof(tile_data));
	dma = {0};
	cgb_colors = cpu->get_mmap().is_cgb_rom();
	printf("is cgb_rom: %u\n", cgb_colors);
	std::memset(cgb_bg_colors, 0, sizeof(cgb_bg_colors));
}

PixelProcessingUnit::~PixelProcessingUnit() {}

void PixelProcessingUnit::tick(uint8_t cycle) {
	// printf("mode: %u clock: %u cycle: %u\n", l_status.mode, lcd_clock, cycle);
	if (ctrl.lcd_enable) {

		lcd_clock += (uint16_t)cycle;
		switch (l_status.mode) {
		case PPU_Modes::Horizontal_Blank:
			if (lcd_clock >= 204) {
				lcd_clock -= 204;
				ly += 1;
				if (ly == 144) {
					l_status.mode = PPU_Modes::Vertical_Blank;
					handle_interrupt(true);
					draw_screen = true;
				} else {
					l_status.mode = PPU_Modes::OAM_Scan;
					handle_interrupt(true);
					if (ctrl.window_enable && window_y == ly) {
						window_active = true;
					}
				}
			}
			break;
		case PPU_Modes::Vertical_Blank:
			if (lcd_clock >= 456) {
				lcd_clock -= 456;
				ly += 1;
				if (ly > 153) {
					window_line_active = 0;
					ly = 0;
					l_status.mode = PPU_Modes::OAM_Scan;
					handle_interrupt(true);
					window_active = ctrl.window_enable && window_y == ly;
				}
			}
			break;
		case PPU_Modes::OAM_Scan:
			dma_transfer(cycle);
			if (lcd_clock >= 80) {
				l_status.mode = PPU_Modes::Pixel_Drawing;
				lcd_clock -= 80;
			}
			break;
		case PPU_Modes::Pixel_Drawing:
			if (lcd_clock >= 172) {
				l_status.mode = PPU_Modes::Horizontal_Blank;
				lcd_clock -= 172;
				render_scanline();
				handle_interrupt(false);
			}
			break;

		default:
			break;
		}
	}
}

void PixelProcessingUnit::dma_transfer(uint8_t cycle) {
	uint16_t src = (dma.val << 8) + dma.offset;
	uint8_t nbytes = (cycle / 4);

	for (uint8_t v = 0; v < nbytes && dma.cycles; v++, dma.cycles -= 4) {
		uint8_t value = cpu->get_mmap().read_u8(src + v);
		write_oam(dma.offset++, value);
	}
}

void PixelProcessingUnit::handle_interrupt(bool val) {
	if (l_status.mode == PPU_Modes::Vertical_Blank) {
		cpu->set_interrupt(InterruptType::Vblank);
	}
	if ((l_status.val & 0x8) && ly == lyc && val) {
		cpu->set_interrupt(InterruptType::Stat);
	}
	if (l_status.mode != PPU_Modes::Pixel_Drawing && (static_cast<uint8_t>(1 << l_status.mode) & l_status.val)) {
		cpu->set_interrupt(InterruptType::Stat);
	}
}

// TODO seems to be an issue here (see one of the sprites in Pokemon blue if you move have a line in it)
void PixelProcessingUnit::handle_sprites(std::vector<std::reference_wrapper<Sprite>> sprites, uint32_t i,
                                         uint8_t tile_data_pos, uint32_t *framebuffer_ptr, size_t *spr_index) {
	bool drawn = false;
	for (auto spr = sprites.begin() + *spr_index; !sprites.empty() && spr != sprites.end(); ++spr) {
		if (!(spr->get().x_pos - 8 <= (int)(i) && spr->get().x_pos > (int)(i))) {
			break;
		}
		// Go to next sprite if this x marks the end of it
		if (i + 1 == spr->get().x_pos)
			(*spr_index)++;
		if (!drawn) {
			uint8_t tile_x = i - (spr->get().x_pos - 8);
			uint8_t tile_y = ly - (spr->get().y_pos - 16);

			if (spr->get().attributes.x_flip)
				tile_x = 7 - tile_x;
			if (spr->get().attributes.y_flip)
				tile_y = (ctrl.obj_size ? 15 : 7) - tile_y;

			uint8_t sprite_index = spr->get().tile_index;

			if (ctrl.obj_size) {
				sprite_index = (sprite_index & 0xFE) + (tile_y >= 8);
				tile_y %= 8;
			}

			uint8_t color_index = tile_data[vbank_select][sprite_index][(uint16_t)tile_y * 8 + tile_x];

			bool background = (spr->get().attributes.background);
			if (color_index && (!background || (background && !tile_data_pos))) {
				if (spr->get().attributes.palette) {
					*framebuffer_ptr = obj_1_colors[color_index];
				} else {
					*framebuffer_ptr = obj_0_colors[color_index];
				}
				drawn = true;
			}
		}
	}
}

void PixelProcessingUnit::render_scanline() {
	std::vector<std::reference_wrapper<Sprite>> sprites_sorted;

	if (ctrl.obj_enable) {
		sprites_sorted.reserve(10);
		for (Sprite &spr : sprites) {
			if (ly < spr.y_pos &&
			    ly >= spr.y_pos - 16) { // obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
				if (ctrl.obj_size || ly < spr.y_pos - 8) {
					sprites_sorted.push_back(spr);
					if (sprites_sorted.size() == 10)
						break;
				}
			}
		}

		// TODO do this in a differnt way
		std::stable_sort(sprites_sorted.begin(), sprites_sorted.end(),
		                 [](const std::reference_wrapper<Sprite> &a, const std::reference_wrapper<Sprite> &b) {
			                 return (a.get().x_pos < b.get().x_pos);
		                 });
		std::erase_if(sprites_sorted, [](const auto &a) { return (a.get().x_pos <= 0 || a.get().x_pos >= 168); });
	}

	int tile_map_offset = ctrl.bg_tile_map_address ? 0x1c00 : 0x1800;
	int line_offset = scx >> 3;
	uint8_t y = (ly + scy) & 7;
	uint8_t x = scx & 7;

	tile_map_offset += (((ly + scy) & 255) >> 3) << 5;
	if (window_active && window_x == 0 + 7) {
		tile_map_offset = (ctrl.window_tile_map_address ? 0x1c00 : 0x1800) + ((window_line_active >> 3) << 5);
		line_offset = 0;
		y = window_line_active & 7;
		x = 0;
	}

	uint32_t *framebuffer_ptr = framebuffer + ly * 160;
	uint16_t tile_index = vram[vbank_select][tile_map_offset + line_offset];
	if (!ctrl.bg_window_tile_data) {
		tile_index = static_cast<uint16_t>(256 + static_cast<int8_t>(tile_index & 0xFF));
	}

	size_t spr_index = 0;
	for (uint32_t i = 0; i < 160; i++) {
		uint8_t tile_dat = tile_data[vbank_select][tile_index][(uint16_t)(y) * 8 + x];
		if (cgb_colors) {
			uint16_t tile_attr_index = vram[1][tile_map_offset + line_offset];
			uint8_t tile_attr = tile_data[1][tile_attr_index][(uint16_t)(y) * 8 + x];
			uint8_t tile_pal = tile_attr & 0b111;
			// if (tile_dat) {
			// 	printf("tile_dat: %u tile_attr: %u tile_pal: %u\n", tile_dat, tile_attr, tile_pal);
			// }
			*framebuffer_ptr = cgb_bg_colors_other_32[tile_pal][tile_dat];
		} else {
			*framebuffer_ptr = bg_colors[tile_dat];
		}

		handle_sprites(sprites_sorted, i, tile_dat, framebuffer_ptr, &spr_index);
		framebuffer_ptr++;
		x++;
		if (x == 8) {
			x = 0;
			line_offset = (line_offset + 1) & 31;
			tile_index = vram[vbank_select][tile_map_offset + line_offset];
			if (!ctrl.bg_window_tile_data) {
				tile_index = static_cast<uint16_t>(256 + static_cast<int8_t>(tile_index & 0xFF));
			}
		}
	}
	if (window_active) {
		window_line_active++;
	}

	/* TODO CGB COLOR STUFF
	    needs:
	        - color_index (tile_dat?)
	        - palette_nb (Option if None -> Default color) otherwise (bits & 0b111) where bits is from tile_index from
	   bank1?
	        - is_background (if sprite this is false)

	    //Also need to handle mixing of colors (only for background)
	*/
}

uint32_t PixelProcessingUnit::get_cgb_color(uint8_t value1, uint8_t value2) {
	uint16_t color_bytes = ((static_cast<uint16_t>(value2)) << 8) | static_cast<uint16_t>(value1);
	uint8_t red = static_cast<uint8_t>(static_cast<float>(color_bytes & RED_MASK) * 255.0 / 31.0);
	uint8_t green = static_cast<uint8_t>(static_cast<float>((color_bytes & GREEN_MASK) >> 5) * 255.0 / 31.0);
	uint8_t blue = static_cast<uint8_t>(static_cast<float>((color_bytes & BLUE_MASK) >> 10) * 255.0 / 31.0);
	uint32_t color = ((red << 24) | (green << 16) | (blue << 8) | 0xFF);
	// if (color != 0x000000FF || value1 || value2) {
	// 	printf("color: %#012x red: %#04x green: %#04x blue: %#04x color_bytes: %u, values: (%u, %u)\n", color, red,
	// green, blue, color_bytes, value1, value2);
	// }
	return color;
}

void PixelProcessingUnit::render_screen() {
	SDL_RenderPresent(data.renderer);
	SDL_UpdateTexture(data.texture, NULL, framebuffer, 160 * sizeof(uint32_t));
	SDL_RenderClear(data.renderer);
	SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
	SDL_RenderPresent(data.renderer);
}

bool PixelProcessingUnit::init_window() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	} else {
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, 0, &data.window, &data.renderer);
		SDL_SetWindowTitle(data.window, "GBMU");
		if (data.window == NULL || data.renderer == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return false;
		} else {
			data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING,
			                                 SCREEN_WIDTH, SCREEN_HEIGHT);
			if (data.texture == NULL) {
				printf("texture could not be created! SDL_Error: %s\n", SDL_GetError());
				return false;
			}
		}
	}
	return true;
}

void PixelProcessingUnit::close() {
	SDL_DestroyTexture(data.texture);
	data.texture = NULL;
	SDL_DestroyRenderer(data.renderer);
	data.renderer = NULL;
	SDL_DestroyWindow(data.window);
	data.window = NULL;
	SDL_Quit();
}

uint8_t PixelProcessingUnit::read_u8_ppu(uint16_t addr) {
	switch (addr) {
	case 0x8000 ... 0x9FFF:
		return vram[vbank_select][addr & 0x1FFF];
	case 0xFF40:
		return ctrl.val;
	case 0xFF41:
		return l_status.val;
	case 0xFF42:
		return scy;
	case 0xFF43:
		return scx;
	case 0xFF44:
		return ly;
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
	case 0xFF4F:
		return vbank_select;
	case 0xFF70:
		return cgb_colors ? (cpu->get_mmap().read_io_registers(addr) | 0xF8) : 0xFF;
	case 0xFF68:
		return bg_palette_cgb;
	case 0xFF69:
		return bg_color_cgb;
	case 0xFF6A:
		return obj_palette_cgb;
	case 0xFF6B:
		return obj_color_cgb;
	default:
		return 0;
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
	case 0xFF40:
		ctrl.set(val);
		break;
	case 0xFF41:
		l_status.set(val);
		break;
	case 0xFF42:
		scy = val;
		break;
	case 0xFF43:
		scx = val;
		break;
	case 0xFF44:
		ly = val;
		break;
	case 0xFF45:
		lyc = val;
		break;
	case 0xFF46:
		dma.set(val);
		break;
	case 0xFF47:
		bg_palette = val;
		for (int i = 0; i < 4; i++) {
			bg_colors[i] = GB_COLORS[(val >> (i * 2)) & 3];
		}
		break;
	case 0xFF48:
		obj_palette_0 = val;
		for (int i = 0; i < 4; i++) {
			obj_0_colors[i] = GB_COLORS[(val >> (i * 2)) & 3];
		}
		break;
	case 0xFF49:
		obj_palette_1 = val;
		for (int i = 0; i < 4; i++) {
			obj_1_colors[i] = GB_COLORS[(val >> (i * 2)) & 3];
		}
		break;
	case 0xFF4A:
		window_y = val;
		break;
	case 0xFF4B:
		window_x = val;
		break;
	case 0xFF4F:
		if (cgb_colors) {
			printf("setting vbank select to: %u val: %u\n", val & 1, val);
			vbank_select = val & 0x01;
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF51:
		if (cgb_colors) {
			set_hdma_register(HDMA_1, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF52:
		if (cgb_colors) {
			set_hdma_register(HDMA_2, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF53:
		if (cgb_colors) {
			set_hdma_register(HDMA_3, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF54:
		if (cgb_colors) {
			set_hdma_register(HDMA_4, val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF55:
		if (cgb_colors) {
			switch_cgb_dma(val);
		} else {
			cpu->get_mmap().write_io_registers(addr, val);
		}
		break;
	case 0xFF68:
		if (cgb_colors) {
			bg_palette_cgb = val;
			update_palette_cgb(true, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF69:
		if (cgb_colors) {
			set_color_palette(true, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF6A:
		if (cgb_colors) {
			obj_palette_cgb = val;
			update_palette_cgb(false, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF6B:
		if (cgb_colors) {
			obj_color_cgb = val;
			set_color_palette(false, val);
		}
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF6C:
		cpu->get_mmap().write_io_registers(addr, val | 0xFE);
		break;
	case 0xFF70:
		if (cgb_colors) {
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
		break;
	}
}

void PixelProcessingUnit::write_oam(uint16_t addr, uint8_t val) {
	uint16_t sprite_addr = addr & 0xFF;
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
	switch (sprite_addr & 3) {
	case 0:
		return sprites[sprite_addr >> 2].y_pos;
	case 1:
		return sprites[sprite_addr >> 2].x_pos;
	case 2:
		return sprites[sprite_addr >> 2].tile_index;
	case 3:
		return sprites[sprite_addr >> 2].attributes.get();
	default:
		return 0xFF;
	}
}

void PixelProcessingUnit::update_palette_cgb(bool background, uint8_t val) {
	bool hl = val & mask0;
	int index = (val >> 1) & 0x03;
	int pal = (val >> 3) & 0x07;

	uint16_t color = background ? cgb_bg_colors_other[pal][index][0] : cgb_sprite_colors_other[pal][index][0];
	if (background) {
		bg_color_cgb = hl ? (color >> 8) & 0xFF : color & 0xFF;
	}
	else {
		obj_color_cgb = hl ? (color >> 8) & 0xFF : color & 0xFF;
	}
	// write_u8_ppu(0xFF69, hl ? (color >> 8) & 0xFF : color & 0xFF);
}

void PixelProcessingUnit::set_color_palette(bool background, uint8_t val) {
	uint8_t ps = background ? bg_palette_cgb : obj_palette_cgb;
	bool hl = ps & mask0;
	int index = (ps >> 1) & 0x03;
	int pal = (ps >> 3) & 0x07;
	bool increment = ps & mask7;

	if (increment) {
		uint8_t addr = ps & 0x3F;
		addr++;
		addr &= 0x3F;
		ps = (ps & 0x80) | addr;
		if (background) {
			bg_palette_cgb = ps;
		}
		else {
			obj_palette_cgb = ps;
		}
		update_palette_cgb(background, ps);
	}

	uint16_t *pa_gbc = background ? &cgb_bg_colors_other[pal][index][0] : &cgb_sprite_colors_other[pal][index][0];
	uint16_t *pa_final = background ? &cgb_bg_colors_other[pal][index][1]: &cgb_sprite_colors_other[pal][index][1];
	uint32_t *pa_32 = background ? &cgb_bg_colors_other_32[pal][index] : &cgb_sprite_colors_other_32[pal][index];
	*pa_gbc = hl ? (*pa_gbc & 0x00FF) | (val << 8) : (*pa_gbc & 0xFF00) | val;
	uint8_t red_5bit = *pa_gbc & 0x1F;
	uint8_t blue_5bit = (*pa_gbc >> 10) & 0x1F;
	uint8_t green_5bit = (*pa_gbc >> 5) & 0x1F;
	// *pa_final = 0x8000 | (red_5bit << 10) | (green_5bit << 5) | blue_5bit;
	uint8_t green_6bit = (*pa_gbc >> 4) & 0x3E;
	*pa_final = (blue_5bit << 11) | (green_6bit << 5) | red_5bit;
	uint8_t red = static_cast<uint8_t>(static_cast<float>(red_5bit) * 255.0 / 31.0);
	// uint8_t green = static_cast<uint8_t>(static_cast<float>(green_5bit) * 255.0 / 31.0);
	uint8_t green = static_cast<uint8_t>(static_cast<float>(green_6bit) * 255.0 / 62.0);
	uint8_t blue = static_cast<uint8_t>(static_cast<float>(blue_5bit) * 255.0 / 31.0);
	*pa_32 = red << 24 | green << 16 | blue << 8 | 0xFF;
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

void PixelProcessingUnit::switch_cgb_dma(uint8_t value) {
	hdma_bytes = 16 + ((value & 0x7f) * 16);

	if (hdma_enable) {
		if (value & mask7) {
			hdma[4] = value & 0x7F;
		} else {
			hdma[4] = 0xFF;
			hdma_enable = false;
		}
	} else {
		if (value & mask7) {
			hdma_enable = true;
			hdma[4] = value & 0x7F;
			if (l_status.mode == PPU_Modes::Horizontal_Blank) {
				cpu->set_cycle(perform_hdma());
			}
		} else {
			perform_gdma(value);
		}
	}
}
void PixelProcessingUnit::set_hdma_register(HDMA_Register reg, uint8_t value) {
	switch (reg) {
	case HDMA_1:
		if (value > 0x7f && value < 0xa0)
			value = 0;
		hdma_source = (value << 8) | (hdma_source & 0xF0);
		break;
	case HDMA_2:
		value &= 0xF0;
		hdma_source = (hdma_source & 0xFF00) | value;
		break;
	case HDMA_3:
		value &= 0x1F;
		hdma_dest = (value << 8) | (hdma_dest & 0xF0);
		hdma_dest |= 0x8000;
		break;
	case HDMA_4:
		value &= 0xF0;
		hdma_dest = (hdma_dest & 0x1F00) | value;
		hdma_dest |= 0x8000;
		break;
	}
}

// This is the gameboy color Horizontal Blanking DMA
uint8_t PixelProcessingUnit::perform_hdma() {
	uint16_t source = hdma_source & 0xFFF0;
	uint16_t destination = (hdma_dest & 0x1FF0) | 0x8000;

	for (int i = 0; i < 0x10; i++) {
		write_u8_ppu(destination + i, cpu->get_mmap().read_u8(source + i));
	}

	hdma_dest += 0x10;
	if (hdma_dest == 0xA000)
		hdma_dest = 0x8000;

	hdma_source += 0x10;
	if (hdma_source == 0x8000)
		hdma_source = 0xA000;

	hdma[1] = hdma_source & 0xFF;
	hdma[0] = hdma_source >> 8;
	hdma[3] = hdma_dest & 0xFF;
	hdma[2] = hdma_dest >> 8;
	hdma_bytes -= 0x10;
	hdma[4]--;

	if (hdma[4] == 0xFF)
		hdma_enable = false;

	return (cpu->get_cgb_speed() ? 17 : 9);
}

// This is the gameboy color general purpose DMA
void PixelProcessingUnit::perform_gdma(uint8_t value) {
	uint16_t source = hdma_source & 0xFFF0;
    uint16_t destination = (hdma_dest & 0x1FF0) | 0x8000;

        for (int i = 0; i < hdma_bytes; i++) {
			write_u8_ppu(destination + i, cpu->get_mmap().read_u8(source + i));
		}

    hdma_dest += hdma_bytes;
    hdma_source += hdma_bytes;

    for (int i = 0; i < 5; i++)
        hdma[i] = 0xFF;

    uint16_t clock_cycles = 0;

    if (cpu->get_cgb_speed())
        clock_cycles = 2 + 16 * ((value & 0x7f) + 1);
    else
        clock_cycles = 1 + 8 * ((value & 0x7f) + 1);

	cpu->set_cycle_16(clock_cycles);
}

void LCD_CONTROL::set(uint8_t value) {
	lcd_enable = (value >> 7) & 1;
	window_tile_map_address = (value >> 6) & 1;
	window_enable = (value >> 5) & 1;
	bg_window_tile_data = (value >> 4) & 1;
	bg_tile_map_address = (value >> 3) & 1;
	obj_size = (value >> 2) & 1;
	obj_enable = (value >> 1) & 1;
	bg_enable = (value >> 0) & 1;
	val = value;
}

void LCD_STATUS::set(uint8_t value) {
	ly_interrupt = (value & 0x40) != 0;
	mode_2_oam_interrupt = (value & 0x20) != 0;
	mode_1_vblank_interrupt = (value & 0x10) != 0;
	mode_0_hblank_interrupt = (value & 0x08) != 0;
	val = value;
}

void LCD_DMA::set(uint8_t value) {
	val = value;
	cycles = 640; // TODO 160 M-cycles: 640 dots (1.4 lines) in normal speed, or 320 dots
	offset = 0;
}

void Sprite_Attributes::set(uint8_t value) {
	background = (value >> 7) & 1;
	y_flip = (value >> 6) & 1;
	x_flip = (value >> 5) & 1;
	palette = (value >> 4) & 1;
}

uint8_t Sprite_Attributes::get() {
	return (background << 7) | (y_flip << 6) | (x_flip << 5) | (palette << 4);
}