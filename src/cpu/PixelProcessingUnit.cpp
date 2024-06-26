#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

PixelProcessingUnit::PixelProcessingUnit(Cpu *cpu) : cpu(cpu) {
	lcd_clock = 0;
	init_window();
	data.status = false;
	ctrl = {0};
	l_status = {0};
	l_status.mode = PPU_Modes::Pixel_Drawing;
	std::memset(oam, 0, sizeof(oam));
	std::memset(framebuffer, 0, sizeof(framebuffer));
	std::memset(vram, 0, sizeof(vram));
	std::memset(obj_0_colors, 0, sizeof(obj_0_colors));
	std::memset(obj_1_colors, 0, sizeof(obj_1_colors));
	std::memset(tile_data, 0, sizeof(tile_data));
	dma = {0};
	cgb_colors = cpu->get_mmap().is_cgb_rom();
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

//TODO seems to be an issue here (see one of the sprites in Pokemon blue if you move have a line in it)
void PixelProcessingUnit::handle_sprites(std::vector<Sprite> sprites, uint32_t i, uint8_t tile_data_pos,
                                         uint32_t *framebuffer_ptr, size_t *spr_index) {
	bool drawn = false;
	for (auto spr = sprites.begin() + *spr_index; !sprites.empty() && spr != sprites.end(); ++spr) {
		if (!(spr->x_pos - 8 <= (int)(i) && spr->x_pos > (int)(i))) {
			break;
		}
		// Go to next sprite if this x marks the end of it
		if (i + 1 == spr->x_pos)
			(*spr_index)++;
		if (!drawn) {
			uint8_t tile_x = i - (spr->x_pos - 8);
			uint8_t tile_y = ly - (spr->y_pos - 16);

			if ((spr->att_flags >> 5) & 1)
				tile_x = 7 - tile_x;
			if ((spr->att_flags >> 6) & 1)
				tile_y = (ctrl.obj_size ? 15 : 7) - tile_y;

			uint8_t sprite_index = spr->tile_index;

			if (ctrl.obj_size) {
				sprite_index = (sprite_index & 0xFE) + (tile_y >= 8);
				tile_y %= 8;
			}

			uint8_t color_index = tile_data[vbank_select][sprite_index][(uint16_t)tile_y * 8 + tile_x];

			bool background = ((spr->att_flags >> 7) & 1);
			if (color_index && (!background || (background && !tile_data_pos))) {
				if ((spr->att_flags >> 4) & 1) {
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
	std::vector<Sprite> sprites;

	if (ctrl.obj_enable) {
		sprites.reserve(10);
		for (int i = 0; i < 40; i++) {
			Sprite spr = get_sprite(i);
			if (ly < spr.y_pos &&
			    ly >= spr.y_pos - 16) { // obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
				if (ctrl.obj_size || ly < spr.y_pos - 8) {
					sprites.push_back(spr);
					if (sprites.size() == 10)
						break;
				}
			}
		}

		// TODO do this in a differnt way
		std::stable_sort(sprites.begin(), sprites.end(), [](const Sprite a, Sprite b) { return (a.x_pos < b.x_pos); });
		std::erase_if(sprites, [](const auto &a) { return (a.x_pos <= 0 || a.x_pos >= 168); });
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
		}
		else {
			*framebuffer_ptr = bg_colors[tile_dat];
		}

		handle_sprites(sprites, i, tile_dat, framebuffer_ptr, &spr_index);
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
			- palette_nb (Option if None -> Default color) otherwise (bits & 0b111) where bits is from tile_index from bank1?
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
	// 	printf("color: %#012x red: %#04x green: %#04x blue: %#04x color_bytes: %u, values: (%u, %u)\n", color, red, green, blue, color_bytes, value1, value2);
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
		return wram_bank_select;
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
			vbank_select = val & 1;
		}
		else {
			vbank_select = 0;
		}
		break;
	case 0xFF68:
		bg_palette_cgb = val;
		break;
	case 0xFF69:
		// bg_color_cgb = val;
		// cgb_bg_colors[(bg_palette_cgb & (PALETTE_SIZE - 1))] = val;
		// bool hl = bg_palette_cgb & mask0;
		// // printf("cgb_bg_colors[%u]: %u\n", (bg_palette_cgb & (64 - 1)), val);
		// if ((bg_palette_cgb & AUTO_INC) != 0) {
		// 	if (bg_palette_cgb == (AUTO_INC | SPEC_INDEX)) {
		// 		bg_palette_cgb = AUTO_INC;
		// 	}
		// 	else {
		// 		bg_palette_cgb +=1;
		// 	}
		// }
		// int index = (bg_palette_cgb >> 1) & 0x03;
		// int pal = (bg_palette_cgb >> 3) & 0x07;
		// uint32_t *pa_gbc = &cgb_bg_colors_other[pal][index][0];
		// uint32_t *pa_final = &cgb_bg_colors_other[pal][index][1];

		// *pa_gbc = hl ? (*pa_gbc & 0x00FF) | (val << 8) : (*pa_gbc & 0xFF00) | val;
		// uint8_t red_5bit = *pa_gbc & 0x1F;
		// uint8_t blue_5bit = (*pa_gbc >> 10) & 0x1F;
		// uint8_t green_5bit = (*pa_gbc >> 5) & 0x1F;
		// *pa_final = red_5bit << 24 | green_5bit << 16 | blue_5bit << 8 | 0xFF; 
		set_cgb_bg_palette(val);
		break;
	case 0xFF6A:
		obj_palette_cgb = val;
		break;
	case 0xFF6B:
		obj_color_cgb = val;
		break;
	case 0xFF70:
		if (cgb_colors) {
			wram_bank_select = ((val & 4) + (val & 2) + (val & 1)); //TODO check this
		}
		else {
			wram_bank_select = 0;
		}
		break;
	default:
		break;
	}
}

void PixelProcessingUnit::write_oam(uint16_t addr, uint8_t val) {
	//TODO also change the sprites here?
	oam[addr / 4][addr % 4] = val;
}

uint8_t PixelProcessingUnit::read_oam(uint16_t addr) {
	return oam[addr / 4][addr % 4];
}

Sprite PixelProcessingUnit::get_sprite(size_t index) {
	uint8_t y_pos = oam[index][0];
	uint8_t x_pos = oam[index][1];
	uint8_t tile_index = oam[index][2];
	uint8_t attr_flags = oam[index][3];
	return {y_pos, x_pos, tile_index, attr_flags};
}

void PixelProcessingUnit::update_palette_cgb(uint8_t val) {
	bool hl = val & mask0;
    int index = (val >> 1) & 0x03;
    int pal = (val >> 3) & 0x07;

    uint16_t color = cgb_bg_colors_other[pal][index][0];
	bg_color_cgb = hl ? (color >> 8) & 0xFF : color & 0xFF;
	// write_u8_ppu(0xFF69, hl ? (color >> 8) & 0xFF : color & 0xFF);
    // m_pMemory->Load(background ? 0xFF69 : 0xFF6B, hl ? (color >> 8) & 0xFF : color & 0xFF);
}

void PixelProcessingUnit::set_cgb_bg_palette(uint8_t val) {
	uint8_t ps = bg_palette_cgb;
	bool hl = ps & mask0;
	int index = (ps >> 1) & 0x03;
	int pal = (ps >> 3) & 0x07;
	bool increment = ps & mask7;
	bg_color_cgb = val;
	cgb_bg_colors[(bg_palette_cgb & (PALETTE_SIZE - 1))] = val;
	// printf("cgb_bg_colors[%u]: %u\n", (bg_palette_cgb & (64 - 1)), val);
	// if ((bg_palette_cgb & AUTO_INC) != 0) {
	// 	if (bg_palette_cgb == (AUTO_INC | SPEC_INDEX)) {
	// 		bg_palette_cgb = AUTO_INC;
	// 	}
	// 	else {
	// 		bg_palette_cgb +=1;
	// 	}
	// }

	if (increment) {
		uint8_t addr = ps & 0x3F;
		addr++;
		addr &= 0x3F;
		ps = (ps & 0x80) | addr;
		bg_palette_cgb = ps;
		update_palette_cgb(ps);
	}

	uint16_t *pa_gbc = &cgb_bg_colors_other[pal][index][0];
	uint16_t *pa_final = &cgb_bg_colors_other[pal][index][1];
	uint32_t *pa_32 = &cgb_bg_colors_other_32[pal][index];
	uint16_t before = *pa_gbc;
	*pa_gbc = hl ? (*pa_gbc & 0x00FF) | (val << 8) : (*pa_gbc & 0xFF00) | val;
	uint8_t red_5bit = *pa_gbc & 0x1F;
	uint8_t blue_5bit = (*pa_gbc >> 10) & 0x1F;
	uint8_t green_5bit = (*pa_gbc >> 5) & 0x1F;
	*pa_final = 0x8000 | (red_5bit << 10) | (green_5bit << 5) | blue_5bit;
	// uint8_t green_6bit = (*pa_gbc >> 4) & 0x3E;
	// *pa_final = (blue_5bit << 11) | (green_6bit << 5) | red_5bit;
	uint8_t red = static_cast<uint8_t>(static_cast<float>(red_5bit) * 255.0 / 31.0);
	uint8_t green = static_cast<uint8_t>(static_cast<float>(green_5bit) * 255.0 / 31.0);
	uint8_t blue = static_cast<uint8_t>(static_cast<float>(blue_5bit) * 255.0 / 31.0);
	*pa_32 = red << 24 | green << 16 | blue << 8 | 0xFF;
	// uint8_t red = red_5bit >> 
	// printf("red: (%#04x, %#04x) green: (%#04x, %#04x) blue: (%#04x, %#04x)\n", red_5bit * 8, red, green_5bit, green, blue_5bit, blue);
	// printf("red: (%u, %u) green: (%u, %u) blue: (%u, %u)\n", red_5bit * 8, red, green_5bit, green, blue_5bit, blue);
	// *pa_32 = red_5bit << 24 | green_5bit << 16 | blue_5bit << 8 | 0xFF;
	// printf("final: %#10x\n", *pa_32);
	// if (blue_5bit) {
		// printf("set color to: %#010x red: %#04x green: %#04x blue: %#04x\n", *pa_final, red_5bit, green_5bit, blue_5bit);
	// }
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
