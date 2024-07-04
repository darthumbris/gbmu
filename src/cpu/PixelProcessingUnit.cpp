#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"
#include "MemoryMap.hpp"
#include <SDL2/SDL_pixels.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>

PixelProcessingUnit::PixelProcessingUnit(Cpu *cpu) : cpu(cpu) {
	lcd_clock = 0;
	init_window();
	data.status = false;
	ctrl = {0};
	l_status = {0};
	l_status.mode = PPU_Modes::Pixel_Drawing;
	// sprites = {0};
	std::memset(mono_framebuffer, 0, sizeof(mono_framebuffer));
	std::memset(rgb_framebuffer, 0, sizeof(rgb_framebuffer));
	std::memset(r5g6b6_framebuffer, 0, sizeof(r5g6b6_framebuffer));
	std::memset(vram, 0, sizeof(vram));
	std::memset(oam, 0, sizeof(oam));
	// std::memset(obj_0_colors, 0, sizeof(obj_0_colors));
	// std::memset(obj_1_colors, 0, sizeof(obj_1_colors));
	std::memset(tile_data, 0, sizeof(tile_data));
	dma = {0};
	is_cgb = cpu->get_mmap().is_cgb_rom();
	printf("is cgb_rom: %u\n", is_cgb);
	// std::memset(cgb_bg_colors, 0, sizeof(cgb_bg_colors));
}

PixelProcessingUnit::~PixelProcessingUnit() {}

void PixelProcessingUnit::tick(uint16_t &cycle) {
	// printf("mode: %u clock: %u cycle: %u\n", l_status.mode, lcd_clock, cycle);
	lcd_clock += cycle;
	if (lcd_enabled) {

		switch (l_status.mode) {
		case PPU_Modes::Horizontal_Blank:
			handle_hblank(cycle);
			break;
		case PPU_Modes::Vertical_Blank:
			handle_vblank(cycle);
			break;
		case PPU_Modes::OAM_Scan:
			handle_oam(cycle);
			break;
		case PPU_Modes::Pixel_Drawing:
			handle_pixel_drawing(cycle);
			break;

		default:
			break;
		}
	} else {
		if (screen_off_cycles > 0) {
			screen_off_cycles -= cycle;
			if (screen_off_cycles <= 0) {
				screen_off_cycles = 0;
				l_status.mode = PPU_Modes::Horizontal_Blank;
				lcd_enabled = true;
				hide_screen = 3;
				window_line_active = 0;
				lcd_clock = 0;
				lcd_clock_vblank = 0;
				vblank_line = 0;
				pixels_drawn = 0;
				tile_drawn = 0;
				ly = 0;
				interrupt_signal = 0;
				if (l_status.mode_2_oam_interrupt) {
					cpu->set_interrupt(InterruptType::Stat);
					interrupt_signal |= mask2;
				}
				compare_ly();
			}
		} else if (lcd_clock >= 70224) {
			lcd_clock -= 70224;
			draw_screen = true;
		}
	}
}

void PixelProcessingUnit::handle_hblank(uint16_t &cycle) {
	if (lcd_clock >= 204) {
		lcd_clock -= 204;
		ly++;
		compare_ly();
		if (is_cgb && hdma_enable && (!cpu->is_halted() || cpu->interrupt_ready())) {
			uint16_t hcycles = perform_hdma();
			lcd_clock += hcycles;
			cycle += hcycles;
		}

		if (ly == 144) {
			l_status.mode = PPU_Modes::Vertical_Blank;
			vblank_line = 0;
			lcd_clock_vblank = lcd_clock;

			cpu->set_interrupt(InterruptType::Vblank);
			interrupt_signal &= 0x09;

			if (l_status.mode_1_vblank_interrupt) {
				if (!(interrupt_signal & mask0) && !(interrupt_signal & mask3)) {
					cpu->set_interrupt(InterruptType::Stat);
				}
				interrupt_signal |= mask1;
			}
			interrupt_signal &= 0x0E;

			if (hide_screen > 0) {
				hide_screen -= 1;
			} else {
				draw_screen = true;
			}
			window_line_active = 0;

		} else {
			l_status.mode = PPU_Modes::OAM_Scan;
			interrupt_signal &= 0x09;
			if (l_status.mode_2_oam_interrupt) {
				if (interrupt_signal == 0) {
					cpu->set_interrupt(InterruptType::Stat);
				}
				interrupt_signal |= mask2;
			}
			interrupt_signal &= 0x0E;
		}
	}
}

void PixelProcessingUnit::handle_vblank(uint16_t &cycle) {
	lcd_clock_vblank += cycle;
	if (lcd_clock_vblank >= 456) {
		lcd_clock_vblank -= 456;
		vblank_line++;
		if (vblank_line <= 9) {
			ly++;
			compare_ly();
		}
	}
	if (lcd_clock >= 4104 && lcd_clock_vblank >= 4 && ly == 153) {
		ly = 0;
		compare_ly();
	}
	if (lcd_clock >= 4560) {
		lcd_clock -= 4560;
		l_status.mode = PPU_Modes::OAM_Scan;
		interrupt_signal &= 0x07;
		interrupt_signal &= 0x0A;

		if (l_status.mode_2_oam_interrupt) {
			if (interrupt_signal == 0) {
				cpu->set_interrupt(InterruptType::Stat);
			}
			interrupt_signal |= mask2;
		}
		interrupt_signal &= 0x0D;
	}
}

void PixelProcessingUnit::handle_oam(uint16_t &cycle) {
	if (lcd_clock >= 80) {
		l_status.mode = PPU_Modes::Pixel_Drawing;
		lcd_clock -= 80;
		drawn_scanline = false;
		interrupt_signal &= 0x08;
	}
}

void PixelProcessingUnit::handle_pixel_drawing(uint16_t &cycle) {
	if (pixels_drawn < 160) {
		tile_drawn += cycle;
		if (lcd_enabled && ctrl.lcd_enable) {
			while (tile_drawn >= 3) {
				render_background(ly, pixels_drawn);
				pixels_drawn += 4;
				tile_drawn -= 3;
				if (pixels_drawn >= 160) {
					break;
				}
			}
		}
	}

	if (lcd_clock >= 160 && !drawn_scanline) {
		render_scanline(ly);
		drawn_scanline = true;
	}

	if (lcd_clock >= 172) {
		pixels_drawn = 0;
		lcd_clock -= 172;
		tile_drawn = 0;
		l_status.mode = PPU_Modes::Horizontal_Blank;
		interrupt_signal &= 0x08;

		if (l_status.mode_0_hblank_interrupt) {
			if (!(interrupt_signal & mask3)) {
				cpu->set_interrupt(InterruptType::Stat);
			}
			interrupt_signal |= mask0;
		}
	}
}

void PixelProcessingUnit::enable_screen() {
	if (!lcd_enabled) {
		screen_off_cycles = 244;
	}
}
void PixelProcessingUnit::disable_screen() {
	lcd_enabled = false;
	ly = 0;
	l_status.mode = 0;
	lcd_clock = 0;
	lcd_clock_vblank = 0;
	interrupt_signal = 0;
}

void PixelProcessingUnit::dma_transfer(uint8_t cycle) {

	if (is_cgb) {
		uint16_t src = (dma.val << 8);
		if (src < 0xE000) {
			if (src >= 0x8000 && src < 0xA000) {
				for (uint16_t i = 0; i < 0xA0; i++) {
					write_oam(0xFE00 + i, read_cgb_vram(src + i, false));
				}
			} else {
				for (uint16_t i = 0; i < 0xA0; i++) {
					write_oam(0xFE00 + i, cpu->get_mmap().read_u8(src + i));
				}
			}
		}
	} else {
		uint16_t src = (cycle << 8);
		if (src >= 0x8000 && src <= 0xE000) {
			for (uint16_t v = 0; v < 0xA0; v++) {
				uint8_t value = cpu->get_mmap().read_u8(src + v);
				write_oam(0xFE00 + v, value);
			}
		}
	}
}

void PixelProcessingUnit::render_background(uint8_t line, uint8_t pixel) {
	int line_width = line * 160;
	uint8_t y = 0;
	if (is_cgb || ctrl.bg_enable) {
		int offset_x_init = pixel & 0x7;
		int offset_x_end = offset_x_init + 160;
		int screen_tile = pixel >> 3;
		int tile_start_addr = ctrl.bg_window_tile_data ? 0x8000 : 0x8800;
		int map_start_addr = ctrl.bg_tile_map_address ? 0x9C00 : 0x9800;
		uint8_t line_scrolled = line + scy;
		int line_scrolled_32 = (line_scrolled >> 3) << 5;
		int tile_pixel_y = line_scrolled & 0x7;
		int tile_pixel_y_2 = tile_pixel_y << 1;
		int tile_pixel_y_flip_2 = (7 - tile_pixel_y) << 1;

		for (int offset_x = offset_x_init; offset_x < offset_x_end; offset_x++) {
			int screen_pixel_x = (screen_tile << 3) + offset_x;
			uint8_t map_pixel_x = screen_pixel_x + scx;
			int map_tile_x = map_pixel_x >> 3;
			int map_tile_offset_x = map_pixel_x & 0x7;
			uint16_t map_tile_addr = map_start_addr + line_scrolled_32 + map_tile_x;
			int map_tile = 0;

			if (!ctrl.bg_window_tile_data) {
				map_tile = static_cast<int8_t>(read_u8_ppu(map_tile_addr));
				map_tile += 128;
			} else {
				map_tile = read_u8_ppu(map_tile_addr);
			}
			uint8_t cgb_tile_attr = is_cgb ? vram[1][map_tile_addr & 0x1FFF] : 0;
			uint8_t cgb_tile_pal = is_cgb ? (cgb_tile_attr & 0x07) : 0;
			bool cgb_tile_bank = is_cgb ? (cgb_tile_attr & mask3) : false;
			bool cgb_tile_xflip = is_cgb ? (cgb_tile_attr & mask5) : false;
			bool cgb_tile_yflip = is_cgb ? (cgb_tile_attr & mask6) : false;
			int map_tile_16 = map_tile << 4;
			int final_pixely_2 = cgb_tile_yflip ? tile_pixel_y_flip_2 : tile_pixel_y_2;
			int tile_address = tile_start_addr + map_tile_16 + final_pixely_2;

			int pixel_x_in_tile = map_tile_offset_x;

			if (cgb_tile_xflip) {
				pixel_x_in_tile = 7 - pixel_x_in_tile;
			}

			int index = line_width + screen_pixel_x;
			uint16_t addr = tile_address;
			addr &= 0x1FFE;
			y = (addr >> 1) & 7;
			if (cgb_tile_yflip) {
				y = 7 - y;
			}
			uint8_t tile_dat;
			int tile_index = (addr >> 4) & 0x1FF;
			if (cgb_tile_bank) {
				tile_dat = tile_data[1][tile_index][y * 8 + pixel_x_in_tile];
			} else {
				tile_dat = tile_data[vbank_select][tile_index][y * 8 + pixel_x_in_tile];
			}

			color_cache_buffer[index] = tile_dat & 0x03;

			if (is_cgb) {
				bool cgb_tile_priority = (cgb_tile_attr & mask7) && ctrl.bg_enable;
				if (cgb_tile_priority && tile_dat) {
					color_cache_buffer[index] |= mask2;
				}
				r5g6b6_framebuffer[index] = cgb_bg_colors[cgb_tile_pal][tile_dat][1];
			} else {
				uint8_t color = (bg_palette >> (tile_dat << 1)) & 0x03;
				r5g6b6_framebuffer[index] = mono_framebuffer[index] = color;
			}
		}
	} else {
		for (int x = 0; x < 4; x++) {
			int position = line_width + pixel + x;
			mono_framebuffer[position] = 0;
			color_cache_buffer[position] = 0;
		}
	}
}

void PixelProcessingUnit::render_window(uint8_t line) {
	if (window_line_active > 143) {
		return;
	}
	if (!ctrl.window_enable) {
		return;
	}

	int wx = window_x - 7;
	if (wx > 159)
		return;

	if ((window_y > 143) || (window_y > line))
		return;

	int tiles = ctrl.bg_window_tile_data ? 0x8000 : 0x8800;
	int map = ctrl.window_tile_map_address ? 0x9C00 : 0x9800;
	int lineAdjusted = window_line_active;
	int y_32 = (lineAdjusted >> 3) << 5;
	int pixely = lineAdjusted & 0x7;
	int pixely_2 = pixely << 1;
	int pixely_2_flip = (7 - pixely) << 1;
	int line_width = (line * 160);
	int y = 0;

	for (int x = 0; x < 32; x++) {
		int tile = 0;

		if (tiles == 0x8800) {
			tile = static_cast<int8_t>(read_u8_ppu(map + y_32 + x));
			tile += 128;
		} else {
			tile = read_u8_ppu(map + y_32 + x);
		}

		uint8_t cgb_tile_attr = is_cgb ? read_cgb_vram(map + y_32 + x, true) : 0;
		uint8_t cgb_tile_pal = is_cgb ? (cgb_tile_attr & 0x07) : 0;
		bool cgb_tile_bank = is_cgb ? (cgb_tile_attr & mask3) : false;
		bool cgb_tile_xflip = is_cgb ? (cgb_tile_attr & mask5) : false;
		bool cgb_tile_yflip = is_cgb ? (cgb_tile_attr & mask6) : false;
		int mapOffsetX = x << 3;
		int tile_16 = tile << 4;
		int final_pixely_2 = (is_cgb && cgb_tile_yflip) ? pixely_2_flip : pixely_2;
		int tile_address = tiles + tile_16 + final_pixely_2;

		uint16_t addr = tile_address;
		addr &= 0x1FFE;
		y = (addr >> 1) & 7;
		if (cgb_tile_yflip) {
			y = 7 - y;
		}
		uint8_t tile_dat;
		int tile_index = (addr >> 4) & 0x1FF;

		for (int pixelx = 0; pixelx < 8; pixelx++) {
			int bufferX = (mapOffsetX + pixelx + wx);

			if (bufferX < 0 || bufferX >= SCREEN_WIDTH)
				continue;

			int pixelx_pos = pixelx;

			if (is_cgb && cgb_tile_xflip) {
				pixelx_pos = 7 - pixelx_pos;
			}

			if (cgb_tile_bank) {
				tile_dat = tile_data[1][tile_index][y * 8 + pixelx_pos];
			} else {
				tile_dat = tile_data[vbank_select][tile_index][y * 8 + pixelx_pos];
			}

			int position = line_width + bufferX;
			color_cache_buffer[position] = tile_dat & 0x03;
			if (is_cgb) {
				bool cgb_tile_priority = (cgb_tile_attr & mask7) && ctrl.bg_enable;
				if (cgb_tile_priority && tile_dat) {
					color_cache_buffer[position] |= mask2;
				}
				r5g6b6_framebuffer[position] = cgb_bg_colors[cgb_tile_pal][tile_dat][1];
			} else {
				uint8_t color = (bg_palette >> (tile_dat << 1)) & 0x03;
				r5g6b6_framebuffer[position] = mono_framebuffer[position] = color;
			}
		}
	}
	window_line_active++;
}

void PixelProcessingUnit::render_sprites(uint8_t line) {
	if (!ctrl.obj_enable)
		return;

	int sprite_height = ctrl.obj_size ? 16 : 8;
	int line_width = (line * 160);

	bool visible_sprites[40];
	int sprite_limit = 0;

	for (int sprite = 0; sprite < 40; sprite++) {
		int sprite_4 = sprite << 2;
		int sprite_y = read_oam(0xFE00 + sprite_4) - 16;

		if ((sprite_y > line) || ((sprite_y + sprite_height) <= line)) {
			visible_sprites[sprite] = false;
			continue;
		}

		sprite_limit++;

		visible_sprites[sprite] = sprite_limit <= 10;
	}

	for (int sprite = 39; sprite >= 0; sprite--) {
		if (!visible_sprites[sprite])
			continue;

		int sprite_4 = sprite << 2;
		int sprite_x = read_oam(0xFE00 + sprite_4 + 1) - 8;

		if ((sprite_x < -7) || (sprite_x >= 160))
			continue;

		int sprite_y = read_oam(0xFE00 + sprite_4) - 16;
		int sprite_tile_16 = (read_oam(0xFE00 + sprite_4 + 2) & ((sprite_height == 16) ? 0xFE : 0xFF)) << 4;
		uint8_t sprite_flags = read_oam(0xFE00 + sprite_4 + 3);
		int sprite_pallette = (sprite_flags & mask4) ? 1 : 0;
		uint8_t palette = sprite_pallette ? obj_palette_1 : obj_palette_0;
		bool xflip = (sprite_flags & mask5);
		bool yflip = (sprite_flags & mask6);
		bool aboveBG = (!(sprite_flags & mask7));
		bool cgb_tile_bank = (sprite_flags & mask3);
		int cgb_tile_pal = sprite_flags & 0x07;
		int tiles = 0x8000;
		int pixel_y = yflip ? ((sprite_height == 16) ? 15 : 7) - (line - sprite_y) : line - sprite_y;
		uint8_t byte1 = 0;
		uint8_t byte2 = 0;
		int pixel_y_2 = 0;
		int offset = 0;

		if (sprite_height == 16 && (pixel_y >= 8)) {
			pixel_y_2 = (pixel_y - 8) << 1;
			offset = 16;
		} else
			pixel_y_2 = pixel_y << 1;

		int tile_address = tiles + sprite_tile_16 + pixel_y_2 + offset;

		if (is_cgb && cgb_tile_bank) {
			byte1 = read_cgb_vram(tile_address, true);
			byte2 = read_cgb_vram(tile_address + 1, true);
		} else {
			byte1 = read_u8_ppu(tile_address);
			byte2 = read_u8_ppu(tile_address + 1);
		}

		for (int pixelx = 0; pixelx < 8; pixelx++) {
			int pixel = (byte1 & (0x01 << (xflip ? pixelx : 7 - pixelx))) ? 1 : 0;
			pixel |= (byte2 & (0x01 << (xflip ? pixelx : 7 - pixelx))) ? 2 : 0;

			if (pixel == 0)
				continue;

			int bufferX = (sprite_x + pixelx);

			if (bufferX < 0 || bufferX >= 160)
				continue;

			int position = line_width + bufferX;
			uint8_t color_cache = color_cache_buffer[position];

			if (is_cgb) {
				if ((color_cache & mask2))
					continue;
			} else {
				int sprite_x_cache = sprite_cache_buffer[position];
				if ((color_cache & mask3) && (sprite_x_cache < sprite_x))
					continue;
			}

			if (!aboveBG && (color_cache & 0x03))
				continue;

			color_cache_buffer[position] = (color_cache & mask3);
			sprite_cache_buffer[position] = sprite_x;
			if (is_cgb) {
				r5g6b6_framebuffer[position] = cgb_obj_colors[cgb_tile_pal][pixel][1];
			} else {
				uint8_t color = (palette >> (pixel << 1)) & 0x03;
				r5g6b6_framebuffer[position] = mono_framebuffer[position] = color;
			}
		}
	}
}

void PixelProcessingUnit::render_scanline(uint8_t line) {
	// std::vector<std::reference_wrapper<Sprite>> sprites_sorted;

	// if (ctrl.obj_enable) {
	// 	sprites_sorted.reserve(10);
	// 	for (Sprite &spr : sprites) {
	// 		if (ly < spr.y_pos &&
	// 		    ly >= spr.y_pos - 16) { // obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
	// 			if (ctrl.obj_size || ly < spr.y_pos - 8) {
	// 				sprites_sorted.push_back(spr);
	// 				if (sprites_sorted.size() == 10)
	// 					break;
	// 			}
	// 		}
	// 	}

	// 	// TODO do this in a differnt way
	// 	std::stable_sort(sprites_sorted.begin(), sprites_sorted.end(),
	// 	                 [](const std::reference_wrapper<Sprite> &a, const std::reference_wrapper<Sprite> &b) {
	// 		                 return (a.get().x_pos < b.get().x_pos);
	// 	                 });
	// 	std::erase_if(sprites_sorted, [](const auto &a) { return (a.get().x_pos <= 0 || a.get().x_pos >= 168); });
	// }

	// int tile_map_offset = ctrl.bg_tile_map_address ? 0x1c00 : 0x1800;
	// int line_offset = scx >> 3;
	// uint8_t y = (ly + scy) & 7;
	// uint8_t x = scx & 7;

	// tile_map_offset += (((ly + scy) & 255) >> 3) << 5;
	// if (window_active && window_x == 0 + 7) {
	// 	tile_map_offset = (ctrl.window_tile_map_address ? 0x1c00 : 0x1800) + ((window_line_active >> 3) << 5);
	// 	line_offset = 0;
	// 	y = window_line_active & 7;
	// 	x = 0;
	// }

	// uint32_t *framebuffer_ptr = framebuffer + ly * 160;
	// uint16_t tile_index = vram[vbank_select][tile_map_offset + line_offset];
	// if (!ctrl.bg_window_tile_data) {
	// 	tile_index = static_cast<uint16_t>(256 + static_cast<int8_t>(tile_index & 0xFF));
	// }

	// size_t spr_index = 0;
	// for (uint32_t i = 0; i < 160; i++) {
	// 	uint8_t tile_dat = tile_data[vbank_select][tile_index][(uint16_t)(y) * 8 + x];
	// 	if (is_cgb) {
	// 		uint16_t tile_attr_index = vram[1][tile_map_offset + line_offset];
	// 		uint8_t tile_attr = tile_data[1][tile_attr_index][(uint16_t)(y) * 8 + x];
	// 		uint8_t tile_pal = tile_attr & 0b111;
	// 		// if (tile_dat) {
	// 		// 	printf("tile_dat: %u tile_attr: %u tile_pal: %u\n", tile_dat, tile_attr, tile_pal);
	// 		// }
	// 		*framebuffer_ptr = cgb_bg_colors_other_32[tile_pal][tile_dat];
	// 	} else {
	// 		*framebuffer_ptr = bg_colors[tile_dat];
	// 	}

	// 	handle_sprites(sprites_sorted, i, tile_dat, framebuffer_ptr, &spr_index);
	// 	framebuffer_ptr++;
	// 	x++;
	// 	if (x == 8) {
	// 		x = 0;
	// 		line_offset = (line_offset + 1) & 31;
	// 		tile_index = vram[vbank_select][tile_map_offset + line_offset];
	// 		if (!ctrl.bg_window_tile_data) {
	// 			tile_index = static_cast<uint16_t>(256 + static_cast<int8_t>(tile_index & 0xFF));
	// 		}
	// 	}
	// }
	// if (window_active) {
	// 	window_line_active++;
	// }

	/* TODO CGB COLOR STUFF
	    needs:
	        - color_index (tile_dat?)
	        - palette_nb (Option if None -> Default color) otherwise (bits & 0b111) where bits is from tile_index from
	   bank1?
	        - is_background (if sprite this is false)

	    //Also need to handle mixing of colors (only for background)
	*/

	if (lcd_enabled && ctrl.lcd_enable) {
		render_background(line, 0);
		render_window(line);
		render_sprites(line);
	} else {
		int line_width = (line * 160);
		if (is_cgb) {
			for (int x = 0; x < 160; x++)
				r5g6b6_framebuffer[line_width + x] = 0x8000;
		} else {
			for (int x = 0; x < 160; x++)
				mono_framebuffer[line_width + x] = 0;
		}
	}
}

void PixelProcessingUnit::render_screen() {
	if (!is_cgb) {
		for (int i = 0; i < SCREEN_PIXELS; i++) {
			r5g6b6_framebuffer[i] = GB_COLORS_ORIGNAL[mono_framebuffer[i]];
		}
	}
	for (int i = 0; i < SCREEN_PIXELS; i++) {
		rgb_framebuffer[i].red = (((r5g6b6_framebuffer[i] >> 11) & 0x1F) * 255 + 15) / 31;
		rgb_framebuffer[i].green = (((r5g6b6_framebuffer[i] >> 5) & 0x3F) * 255 + 31) / 63;
		rgb_framebuffer[i].blue = ((r5g6b6_framebuffer[i] & 0x1F) * 255 + 15) / 31;

		if (is_cgb) {
			uint8_t red = (uint8_t)(((rgb_framebuffer[i].red * 0.8125f) + (rgb_framebuffer[i].green * 0.125f) +
			                         (rgb_framebuffer[i].blue * 0.0625f)) *
			                        0.95f);
			uint8_t green = (uint8_t)(((rgb_framebuffer[i].green * 0.75f) + (rgb_framebuffer[i].blue * 0.25f)) * 0.95f);
			uint8_t blue = (uint8_t)((((rgb_framebuffer[i].red * 0.1875f) + (rgb_framebuffer[i].green * 0.125f) +
			                           (rgb_framebuffer[i].blue * 0.6875f))) *
			                         0.95f);

			rgb_framebuffer[i].red = red;
			rgb_framebuffer[i].green = green;
			rgb_framebuffer[i].blue = blue;
		}
	}
	SDL_RenderPresent(data.renderer);
	SDL_UpdateTexture(data.texture, NULL, rgb_framebuffer, 160 * sizeof(RGB_COLOR));
	SDL_RenderClear(data.renderer);
	SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
	SDL_RenderPresent(data.renderer);
	draw_screen = false;
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
			data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
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

void PixelProcessingUnit::set_tile_data(uint16_t addr) {
	// printf("setting tile_data for tile_address: %#06x\n", addr);
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

uint8_t PixelProcessingUnit::read_u8_ppu(uint16_t addr) {
	switch (addr) {
	case 0x8000 ... 0x9FFF:
		// printf("trying to read addr: %u\n", addr);
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
		l_status.set(val);
		if (ctrl.lcd_enable) {
			if (l_status.mode_0_hblank_interrupt && current_mode == 0) {
				if (signal == 0) {
					cpu->set_interrupt(InterruptType::Stat);
				}
				signal |= mask0;
			}
			if ((new_stat & mask4) && (current_mode == 1)) {
				if (signal == 0) {
					cpu->set_interrupt(InterruptType::Stat);
				}
				signal |= mask1;
			}
			if ((new_stat & mask5) && (current_mode == 2)) {
				if (signal == 0) {
					cpu->set_interrupt(InterruptType::Stat);
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
		ly = val; // TODO check if this needs to be enabled?
		break;
	case 0xFF45:
		if (lyc != val) {
			lyc = val;
			if (ctrl.lcd_enable) {
				compare_ly();
			}
		}
		// lyc = val; // TODO check if this needs to be enabled?
		break;
	case 0xFF46:
		dma.set(val); // TODO do this
		dma_transfer(val);
		cpu->get_mmap().write_io_registers(addr, val);
		break;
	case 0xFF47:
		bg_palette = val;
		// for (int i = 0; i < 4; i++) {
		// 	bg_colors[i] = GB_COLORS_ORIGNAL[(val >> (i * 2)) & 3];
		// }
		break;
	case 0xFF48:
		obj_palette_0 = val;
		// for (int i = 0; i < 4; i++) {
		// 	obj_0_colors[i] = GB_COLORS_ORIGNAL[(val >> (i * 2)) & 3];
		// }
		break;
	case 0xFF49:
		obj_palette_1 = val;
		// for (int i = 0; i < 4; i++) {
		// 	obj_1_colors[i] = GB_COLORS_ORIGNAL[(val >> (i * 2)) & 3];
		// }
		break;
	case 0xFF4A:
		window_y = val;
		break;
	case 0xFF4B:
		window_x = val;
		break;
	case 0xFF4F:
		if (is_cgb) {
			printf("setting vbank select to: %u val: %u\n", val & 1, val);
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

void PixelProcessingUnit::write_oam(uint16_t addr, uint8_t val) {
	uint16_t sprite_addr = addr & 0xFF;
	// printf("sprite addr: %#06x or: %u\n", sprite_addr, sprite_addr);
	oam[sprite_addr / 4][sprite_addr % 4] = val;
	// switch (sprite_addr & 3) {
	// case 0:
	// 	sprites[sprite_addr >> 2].y_pos = val;
	// 	break;
	// case 1:
	// 	sprites[sprite_addr >> 2].x_pos = val;
	// 	break;
	// case 2:
	// 	sprites[sprite_addr >> 2].tile_index = val;
	// 	break;
	// case 3:
	// 	sprites[sprite_addr >> 2].attributes.set(val);
	// 	break;
	// }
}

uint8_t PixelProcessingUnit::read_oam(uint16_t addr) {
	// printf("reading oam at addr: %#06x\n", addr);
	uint16_t sprite_addr = addr & 0xFF;
	return oam[sprite_addr / 4][sprite_addr % 4];
	// switch (sprite_addr & 3) {
	// case 0:
	// 	return sprites[sprite_addr >> 2].y_pos;
	// case 1:
	// 	return sprites[sprite_addr >> 2].x_pos;
	// case 2:
	// 	return sprites[sprite_addr >> 2].tile_index;
	// case 3:
	// 	return sprites[sprite_addr >> 2].attributes.get();
	// default:
	// 	return 0xFF;
	// }
}

uint8_t PixelProcessingUnit::read_cgb_vram(uint16_t addr, bool force) {
	if (force || (vbank_select == 1)) {
		return vram[1][addr - 0x8000];
	} else {
		printf("reading cgb_vram: %#06x\n", addr);
		return read_u8_ppu(addr);
	}
}

void PixelProcessingUnit::update_palette_cgb(bool background, uint8_t val) {
	bool hl = val & mask0;
	int index = (val >> 1) & 0x03;
	int pal = (val >> 3) & 0x07;

	uint16_t color = background ? cgb_bg_colors[pal][index][0] : cgb_obj_colors[pal][index][0];
	if (background) {
		bg_color_cgb = hl ? (color >> 8) & 0xFF : color & 0xFF;
	} else {
		obj_color_cgb = hl ? (color >> 8) & 0xFF : color & 0xFF;
	}
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
		} else {
			obj_palette_cgb = ps;
		}
		update_palette_cgb(background, ps);
	}

	uint16_t *pa_gbc = background ? &cgb_bg_colors[pal][index][0] : &cgb_obj_colors[pal][index][0];
	uint16_t *pa_final = background ? &cgb_bg_colors[pal][index][1] : &cgb_obj_colors[pal][index][1];
	// uint32_t *pa_32 = background ? &cgb_bg_colors_other_32[pal][index] : &cgb_sprite_colors_other_32[pal][index];
	*pa_gbc = hl ? (*pa_gbc & 0x00FF) | (val << 8) : (*pa_gbc & 0xFF00) | val;
	uint8_t red_5bit = *pa_gbc & 0x1F;
	uint8_t blue_5bit = (*pa_gbc >> 10) & 0x1F;
	uint8_t green_5bit = (*pa_gbc >> 5) & 0x1F;
	// *pa_final = 0x8000 | (red_5bit << 10) | (green_5bit << 5) | blue_5bit;
	uint8_t green_6bit = (*pa_gbc >> 4) & 0x3E;
	*pa_final = (blue_5bit << 11) | (green_6bit << 5) | red_5bit;
	// uint8_t red = static_cast<uint8_t>(static_cast<float>(red_5bit) * 255.0 / 31.0);
	// uint8_t green = static_cast<uint8_t>(static_cast<float>(green_5bit) * 255.0 / 31.0);
	// uint8_t green = static_cast<uint8_t>(static_cast<float>(green_6bit) * 255.0 / 63.0);
	// uint8_t blue = static_cast<uint8_t>(static_cast<float>(blue_5bit) * 255.0 / 31.0);
	// *pa_32 = red << 24 | green << 16 | blue << 8 | 0xFF;
}

// void PixelProcessingUnit::set_tile_data(uint16_t addr) {
// 	addr &= 0x1FFE;

// 	int tile_index = (addr >> 4) & 0x1FF;
// 	int y = (addr >> 1) & 7;

// 	if (tile_index == 384)
// 		return;

// 	for (int x = 0; x < 8; x++) {
// 		unsigned char bitmask = 1 << (7 - x);
// 		tile_data[vbank_select][tile_index][y * 8 + x] = static_cast<uint8_t>(
// 		    ((vram[vbank_select][addr] & bitmask) ? 1 : 0) + ((vram[vbank_select][addr + 1] & bitmask) ? 2 : 0));
// 	}
// }

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

uint8_t PixelProcessingUnit::get_hdma_register(uint8_t reg) {
	return hdma[reg - 1];
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

void PixelProcessingUnit::compare_ly() {
	if (lcd_enabled) {
		if (lyc == ly) {
			l_status.ly_flag = true;
			if (l_status.ly_interrupt) {
				if (interrupt_signal == 0) {
					cpu->set_interrupt(InterruptType::Stat);
				}
				interrupt_signal |= mask3;
			}
		} else {
			l_status.ly_flag = false;
			interrupt_signal &= ~mask3;
		}
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
	ly_interrupt = (value & 0x40) != 0;            // bit 6
	mode_2_oam_interrupt = (value & 0x20) != 0;    // bit 5
	mode_1_vblank_interrupt = (value & 0x10) != 0; // bit 4
	mode_0_hblank_interrupt = (value & 0x08) != 0; // bit 3
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