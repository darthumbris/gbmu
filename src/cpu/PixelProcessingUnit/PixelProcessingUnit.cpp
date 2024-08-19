#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"
#include "Interruptor.hpp"
#include "MemoryMap.hpp"
#include "debug.hpp"
#include <SDL2/SDL_pixels.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

PixelProcessingUnit::PixelProcessingUnit(Cpu *cpu) : cpu(cpu) {
	lcd_clock = 0;
	init_window();
	data.status = false;
	ctrl = {};
	l_status = {};
	l_status.mode = ppu_modes::Vertical_Blank;
	std::memset(mono_framebuffer, 0, sizeof(mono_framebuffer));
	std::memset(rgb555_framebuffer, 0, sizeof(rgb555_framebuffer));
	std::memset(vram, 0, sizeof(vram));
	std::memset(oam, 0, sizeof(oam));
	std::memset(tile_data, 0, sizeof(tile_data));
	dma = {};
	// is_cgb = cpu->get_mmap().is_cgb_rom();
	ly = SCREEN_HEIGHT;
}

PixelProcessingUnit::~PixelProcessingUnit() {}

void PixelProcessingUnit::tick(uint16_t &cycle) {
	lcd_clock += cycle;
	DEBUG_MSG("cycle: %u ", cycle);
	DEBUG_MSG("%u %u %u %u %u %u %u %u %u %u %u\n", scx, scy, ly, vblank_line, lyc, window_x, window_y, lcd_clock,
	          lcd_enabled, l_status.mode, l_status.get());
	if (lcd_enabled) {
		switch (l_status.mode) {
		case ppu_modes::Horizontal_Blank:
			handle_hblank(cycle);
			break;
		case ppu_modes::Vertical_Blank:
			handle_vblank(cycle);
			break;
		case ppu_modes::OAM_Scan:
			handle_oam();
			break;
		case ppu_modes::Pixel_Drawing:
			handle_pixel_drawing(cycle);
			break;
		}
	} else {
		handle_disabled_screen(cycle);
	}
}

void PixelProcessingUnit::handle_disabled_screen(uint16_t &cycle) {
	if (screen_off_cycles > 0) {
		screen_off_cycles -= cycle;
		if (screen_off_cycles <= 0) {
			screen_off_cycles = 0;
			l_status.mode = ppu_modes::Horizontal_Blank;
			lcd_enabled = true;
			hide_screen = 3;
			window_line_active = 0;
			lcd_clock = 0;
			lcd_clock_vblank = 0;
			vblank_line = 0;
			pixels_drawn = 0;
			tile_drawn = 0;
			interrupt_signal = 0;
			if (l_status.mode_2_oam_interrupt) {
				cpu->interrupt().set_interrupt(interrupt_type::Stat);
				interrupt_signal |= mask2;
			}
			reset_ly();
		}
	} else if (lcd_clock >= FRAME_CYCLES) {
		lcd_clock -= FRAME_CYCLES;
		draw_screen = true;
	}
}

void PixelProcessingUnit::handle_hblank(uint16_t &cycle) {
	if (lcd_clock >= HORIZONTAL_BLANK_CYCLES) {
		lcd_clock -= HORIZONTAL_BLANK_CYCLES;
		increase_ly();
		if (is_cgb && hdma_enable && (!cpu->is_halted() || cpu->interrupt().interrupt_ready())) {
			uint16_t hcycles = perform_hdma();
			lcd_clock += hcycles;
			cycle += hcycles;
		}

		if (ly == SCREEN_HEIGHT) {
			l_status.mode = ppu_modes::Vertical_Blank;
			vblank_line = 0;
			lcd_clock_vblank = lcd_clock;
			cpu->interrupt().set_interrupt(interrupt_type::Vblank);
			interrupt_signal &= 0b1001;

			if (l_status.mode_1_vblank_interrupt) {
				if (!(interrupt_signal & mask0) && !(interrupt_signal & mask3)) {
					cpu->interrupt().set_interrupt(interrupt_type::Stat);
				}
				interrupt_signal |= mask1;
			}
			interrupt_signal &= 0b1110;

			if (hide_screen > 0) {
				hide_screen -= 1;
			} else {
				draw_screen = true;
			}
			window_line_active = 0;

		} else {
			l_status.mode = ppu_modes::OAM_Scan;
			interrupt_signal &= 0b1001;
			if (l_status.mode_2_oam_interrupt) {
				if (interrupt_signal == 0) {
					cpu->interrupt().set_interrupt(interrupt_type::Stat);
				}
				interrupt_signal |= mask2;
			}
			interrupt_signal &= 0b1110;
		}
		uint8_t stat = l_status.get();
		l_status.set((stat & 0xFC) | (l_status.mode & 0x3));
		DEBUG_MSG("setting lstat: %u\n", (stat & 0xFC) | (l_status.mode & 0x3));
	}
}

void PixelProcessingUnit::handle_vblank(uint16_t &cycle) {
	lcd_clock_vblank += cycle;
	if (lcd_clock_vblank >= VERTICAL_BLANK_CYCLES) {
		lcd_clock_vblank -= VERTICAL_BLANK_CYCLES;
		vblank_line++;
		if (vblank_line < VBLANK_SCANLINES) {
			increase_ly();
		}
	}
	if (lcd_clock >= SCANLINES_9_CYCLES && lcd_clock_vblank >= 4 && ly == MAX_SCANLINES) {
		reset_ly();
	}
	if (lcd_clock >= SCANLINES_10_CYCLES) {
		lcd_clock -= SCANLINES_10_CYCLES;
		l_status.mode = ppu_modes::OAM_Scan;
		uint8_t stat = l_status.get();
		l_status.set((stat & 0xFC) | (l_status.mode & 0x3));
		DEBUG_MSG("setting lstat: %u\n", (stat & 0xFC) | (l_status.mode & 0x3));

		interrupt_signal &= 0b0111;
		interrupt_signal &= 0b1010;

		if (l_status.mode_2_oam_interrupt) {
			if (interrupt_signal == 0) {
				cpu->interrupt().set_interrupt(interrupt_type::Stat);
			}
			interrupt_signal |= mask2;
		}
		interrupt_signal &= 0b1101;
	}
}

void PixelProcessingUnit::handle_oam() {
	if (lcd_clock >= OAM_CYCLES) {
		lcd_clock -= OAM_CYCLES;
		l_status.mode = ppu_modes::Pixel_Drawing;
		drawn_scanline = false;
		interrupt_signal &= 0b1000;
		uint8_t stat = l_status.get();
		l_status.set((stat & 0xFC) | (l_status.mode & 0x3));
		DEBUG_MSG("setting lstat: %u\n", (stat & 0xFC) | (l_status.mode & 0x3));
	}
}

void PixelProcessingUnit::handle_pixel_drawing(uint16_t &cycle) {
	if (pixels_drawn < SCREEN_WIDTH) {
		tile_drawn += cycle;
		if (lcd_enabled && ctrl.lcd_enable) {
			while (tile_drawn >= 3) {
				render_background(ly, pixels_drawn);
				pixels_drawn += 4;
				tile_drawn -= 3;
				if (pixels_drawn >= SCREEN_WIDTH) {
					break;
				}
			}
		}
	}

	if (lcd_clock >= SCREEN_WIDTH && !drawn_scanline) {
		render_scanline(ly);
		drawn_scanline = true;
	}

	if (lcd_clock >= 172) {
		pixels_drawn = 0;
		lcd_clock -= 172;
		tile_drawn = 0;
		l_status.mode = ppu_modes::Horizontal_Blank;
		interrupt_signal &= 0b1000;
		uint8_t stat = l_status.get();
		l_status.set((stat & 0xFC) | (l_status.mode & 0x3));
		DEBUG_MSG("setting lstat: %u\n", (stat & 0xFC) | (l_status.mode & 0x3));

		if (l_status.mode_0_hblank_interrupt) {
			if (!(interrupt_signal & mask3)) {
				cpu->interrupt().set_interrupt(interrupt_type::Stat);
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
	l_status.set(l_status.val & 0x7c);
	DEBUG_MSG("setting lstat: %u\n", l_status.get());
	lcd_clock = 0;
	lcd_clock_vblank = 0;
	interrupt_signal = 0;
}

void PixelProcessingUnit::render_background_dmg(uint16_t tile_addr, uint16_t index, uint8_t pixel_x) {
	uint16_t tile_address = (tile_addr) & 0x1FFE;
	uint8_t y = (tile_address >> 1) & 7;
	uint8_t tile_dat = tile_data[0][(tile_address >> 4) & 0x1FF][y * 8 + pixel_x];

	color_cache_buffer[index] = tile_dat & 0x03;
	rgb555_framebuffer[index] = mono_framebuffer[index] = (bg_palette >> (tile_dat << 1)) & 0x03;
}

void PixelProcessingUnit::render_background_cgb(uint16_t map_addr, uint16_t tile_addr, uint16_t index, uint8_t pixel_y,
                                                uint8_t pixel_x) {
	uint8_t cgb_tile_attr = vram[1][map_addr & 0x1FFF];
	uint16_t tile_address = (tile_addr + ((cgb_tile_attr & mask6) ? ((7 - pixel_y) << 1) : (pixel_y << 1))) & 0x1FFE;
	uint8_t y = (tile_address >> 1) & 7;

	if (cgb_tile_attr & mask5) {
		pixel_x = 7 - pixel_x;
	}

	uint8_t tile_dat;
	if (cgb_tile_attr & mask3) {
		tile_dat = tile_data[1][(tile_address >> 4) & 0x1FF][y * 8 + pixel_x];
	} else {
		tile_dat = tile_data[0][(tile_address >> 4) & 0x1FF][y * 8 + pixel_x];
	}

	color_cache_buffer[index] = tile_dat & 0x03;
	if ((cgb_tile_attr & mask7) && ctrl.bg_enable && (tile_dat != 0)) {
		color_cache_buffer[index] |= mask2;
	}
	rgb555_framebuffer[index] = cgb_bg_colors[(cgb_tile_attr & 0x07)][tile_dat][1];
}

void PixelProcessingUnit::render_background(uint8_t line, uint8_t pixel) {
	int line_width = line * SCREEN_WIDTH;
	if (is_cgb || ctrl.bg_enable) {
		uint8_t offset_x_init = pixel & 0x7;
		uint16_t tile_addr = ctrl.bg_window_tile_data ? 0x8000 : 0x8800;
		uint16_t map_start_addr = ctrl.bg_tile_map_address ? 0x9C00 : 0x9800;
		uint8_t line_scrolled = line + scy;
		uint16_t line_scrolled_16 = (line_scrolled >> 3) << 5;
		uint8_t pixel_y = line_scrolled & 0x7;

		for (uint8_t offset_x = offset_x_init; offset_x < offset_x_init + PIXELS_TO_RENDER; offset_x++) {
			uint16_t screen_pixel_x = ((pixel >> 3) << 3) + offset_x;
			uint8_t map_pixel_x = screen_pixel_x + scx;
			uint16_t map_tile_x = map_pixel_x >> 3;
			uint16_t map_addr = map_start_addr + line_scrolled_16 + map_tile_x;
			uint16_t map_tile = 0;

			if (tile_addr == 0x8800) {
				map_tile = static_cast<int8_t>(vram[0][map_addr & 0x1FFF]) + 128;
			} else {
				map_tile = vram[0][map_addr & 0x1FFF];
			}
			if (is_cgb) {
				render_background_cgb(map_addr, tile_addr + (map_tile << 4), line_width + screen_pixel_x, pixel_y,
				                      map_pixel_x & 0x7);
			} else {
				render_background_dmg(tile_addr + (map_tile << 4) + (pixel_y << 1), line_width + screen_pixel_x,
				                      map_pixel_x & 0x7);
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

void PixelProcessingUnit::render_window_cgb(uint16_t line_width, uint16_t map_address, int16_t wx) {
	for (uint8_t x = 0; x < 32; x++) {
		int16_t tile = 0;

		if (!ctrl.bg_window_tile_data) {
			tile = static_cast<int8_t>(vram[0][(map_address + x) & 0x1FFF]) + 128;
		} else {
			tile = vram[0][(map_address + x) & 0x1FFF];
		}

		uint8_t cgb_tile_attr = vram[1][(map_address + x) & 0x1FFF];
		uint8_t cgb_tile_pal = cgb_tile_attr & 0x07;
		uint16_t map_offset_x = x << 3;
		uint16_t final_pixely_2 =
		    (cgb_tile_attr & mask6) ? ((7 - (window_line_active & 0x7)) << 1) : ((window_line_active & 0x7) << 1);
		uint16_t tile_address = ((ctrl.bg_window_tile_data ? 0x8000 : 0x8800) + (tile << 4) + final_pixely_2) & 0x1FFE;
		uint8_t tile_dat;
		uint16_t tile_index = (tile_address >> 4) & 0x1FF;
		uint8_t y = (tile_address >> 1) & 7;

		for (uint8_t pixelx = 0; pixelx < 8; pixelx++) {
			int16_t buffer_x = (map_offset_x + pixelx + wx);

			if (buffer_x < 0 || buffer_x >= SCREEN_WIDTH)
				continue;

			if (cgb_tile_attr & mask5) {
				pixelx = 7 - pixelx;
			}

			if (cgb_tile_attr & mask3) {
				tile_dat = tile_data[1][tile_index][y * 8 + pixelx];
			} else {
				tile_dat = tile_data[0][tile_index][y * 8 + pixelx];
			}

			int position = line_width + buffer_x;
			color_cache_buffer[position] = tile_dat & 0x03;
			bool cgb_tile_priority = (cgb_tile_attr & mask7) && ctrl.bg_enable;
			if (cgb_tile_priority && (tile_dat != 0)) {
				color_cache_buffer[position] |= mask2;
			}
			rgb555_framebuffer[position] = cgb_bg_colors[cgb_tile_pal][tile_dat][1];
		}
	}
}

void PixelProcessingUnit::render_window_dmg(uint16_t line_width, uint16_t map_address, int16_t wx) {
	for (uint8_t x = 0; x < 32; x++) {
		int16_t tile = 0;

		if (!ctrl.bg_window_tile_data) {
			tile = static_cast<int8_t>(vram[0][(map_address + x) & 0x1FFF]) + 128;
		} else {
			tile = vram[0][(map_address + x) & 0x1FFF];
		}

		uint16_t map_offset_x = x << 3;
		uint16_t tile_address =
		    ((ctrl.bg_window_tile_data ? 0x8000 : 0x8800) + (tile << 4) + ((window_line_active & 0x7) << 1)) & 0x1FFE;
		uint8_t tile_dat;
		uint16_t tile_index = (tile_address >> 4) & 0x1FF;
		uint8_t y = (tile_address >> 1) & 7;

		for (uint8_t pixelx = 0; pixelx < 8; pixelx++) {
			int16_t buffer_x = (map_offset_x + pixelx + wx);

			if (buffer_x < 0 || buffer_x >= SCREEN_WIDTH)
				continue;

			tile_dat = tile_data[0][tile_index][y * 8 + pixelx];

			int position = line_width + buffer_x;
			color_cache_buffer[position] = tile_dat & 0x03;
			uint8_t color = (bg_palette >> (tile_dat << 1)) & 0x03;
			rgb555_framebuffer[position] = mono_framebuffer[position] = color;
		}
	}
}

void PixelProcessingUnit::render_window(uint8_t line) {
	int16_t wx = window_x - 7;
	if (window_line_active > 143 || !ctrl.window_enable || wx > 159 || (window_y > 143) || (window_y > line)) {
		return;
	}

	uint16_t map = (ctrl.window_tile_map_address ? 0x9C00 : 0x9800) + ((window_line_active >> 3) << 5);
	uint16_t line_width = line * SCREEN_WIDTH;
	if (is_cgb) {
		render_window_cgb(line_width, map, wx);
	} else {
		render_window_dmg(line_width, map, wx);
	}
	window_line_active++;
}

void PixelProcessingUnit::render_sprites(uint8_t line) {
	if (!ctrl.obj_enable)
		return;

	uint8_t sprite_height = ctrl.obj_size ? 16 : 8;
	uint16_t line_width = line * SCREEN_WIDTH;
	bool visible_sprites[40];
	uint8_t sprite_limit = 0;
	uint8_t y = 0;

	for (uint8_t i = 0; i < 40; i++) {
		int16_t sprite_y = read_oam(i << 2) - 16;

		if ((sprite_y > line) || ((sprite_y + sprite_height) <= line)) {
			visible_sprites[i] = false;
			continue;
		}
		sprite_limit++;
		visible_sprites[i] = sprite_limit <= 10;
	}

	for (int16_t i = 39; i >= 0; i--) {
		if (!visible_sprites[i])
			continue;

		sprite spr = read_sprite(i << 2);
		int16_t sprite_x = spr.x_pos - 8;

		if ((sprite_x < -7) || (sprite_x >= SCREEN_WIDTH))
			continue;

		sprite_attributes atr = spr.attributes;
		uint8_t palette = atr.palette ? obj_palette_1 : obj_palette_0;
		uint16_t pixel_y =
		    atr.y_flip ? ((sprite_height == 16) ? 15 : 7) - (line - (spr.y_pos - 16)) : line - (spr.y_pos - 16);

		if (sprite_height == 16 && (pixel_y >= 8)) {
			pixel_y = ((pixel_y - 8) << 1) + 16;
		} else
			pixel_y = pixel_y << 1;

		uint16_t tile_address = ((spr.tile_index & ((sprite_height == 16) ? 0xFE : 0xFF)) << 4) + pixel_y;

		y = (tile_address >> 1) & 7;

		// TODO check if this is indeed unnecesary
		//  if (atr.y_flip) {
		//  	y = 7 - y;
		//  }

		uint8_t tile_dat;
		uint16_t tile_index = (tile_address >> 4) & 0x1FF;
		for (uint8_t pixelx = 0; pixelx < 8; pixelx++) {
			if (atr.bank) {
				tile_dat = tile_data[1][tile_index][y * 8 + (!atr.x_flip ? pixelx : (7 - pixelx))];
			} else {
				tile_dat = tile_data[vbank_select][tile_index][y * 8 + (!atr.x_flip ? pixelx : (7 - pixelx))];
			}
			if (tile_dat == 0)
				continue;

			int16_t bufferX = (sprite_x + pixelx);

			if (bufferX < 0 || bufferX >= SCREEN_WIDTH)
				continue;

			int position = line_width + bufferX;
			uint8_t color_cache = color_cache_buffer[position];

			if (is_cgb && (color_cache & mask2)) {
				continue;
			} else if (!is_cgb && (color_cache & mask3) && sprite_cache_buffer[position] < sprite_x) {
				continue;
			}

			if (atr.background && (color_cache & 0x03))
				continue;

			color_cache_buffer[position] = color_cache & mask3;
			sprite_cache_buffer[position] = sprite_x;
			if (is_cgb) {
				rgb555_framebuffer[position] = cgb_obj_colors[atr.cgb_pal][tile_dat][1];
			} else {
				uint8_t color = (palette >> (tile_dat << 1)) & 0x03;
				rgb555_framebuffer[position] = mono_framebuffer[position] = color;
			}
		}
	}
}

void PixelProcessingUnit::render_scanline(uint8_t line) {
	if (lcd_enabled && ctrl.lcd_enable) {
		render_window(line);
		render_sprites(line);
	} else {
		int line_width = line * SCREEN_WIDTH;
		if (is_cgb) {
			for (int x = 0; x < SCREEN_WIDTH; x++)
				rgb555_framebuffer[line_width + x] = 0x8000;
		} else {
			for (int x = 0; x < SCREEN_WIDTH; x++)
				mono_framebuffer[line_width + x] = 0;
		}
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

	uint16_t *palette_gbc = background ? &cgb_bg_colors[pal][index][0] : &cgb_obj_colors[pal][index][0];
	uint16_t *palette_final = background ? &cgb_bg_colors[pal][index][1] : &cgb_obj_colors[pal][index][1];
	*palette_gbc = hl ? (*palette_gbc & 0x00FF) | (val << 8) : (*palette_gbc & 0xFF00) | val;
	uint8_t red_5bit = *palette_gbc & 0x1F;
	// uint8_t green_6bit = (*palette_gbc >> 4) & 0x3E;
	uint8_t green_5bit = (*palette_gbc >> 5) & 0x1F;
	uint8_t blue_5bit = (*palette_gbc >> 10) & 0x1F;
	// *palette_final = (red_5bit << 11) | (green_6bit << 5) | blue_5bit;
	*palette_final = 0x8000 | (red_5bit << 10) | (green_5bit << 5) | blue_5bit;
}

void PixelProcessingUnit::reset_ly() {
	ly = 0;
	compare_ly();
}

void PixelProcessingUnit::increase_ly() {
	ly += 1;
	compare_ly();
}

void PixelProcessingUnit::compare_ly() {
	if (lcd_enabled) {
		DEBUG_MSG("before lstat compare: %u\n", l_status.get());
		if (lyc == ly) {
			l_status.ly_flag = true;
			if (l_status.ly_interrupt) {
				if (interrupt_signal == 0) {
					cpu->interrupt().set_interrupt(interrupt_type::Stat);
				}
				interrupt_signal |= mask3;
			}
		} else {
			l_status.ly_flag = false;
			interrupt_signal &= ~mask3;
		}
		DEBUG_MSG("setting lstat compare: %u\n", l_status.get());
	}
}

void lcd_control::set(uint8_t value) {
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

void lcd_status::set(uint8_t value) {
	ly_interrupt = (value & 0x40) != 0;            // bit 6
	mode_2_oam_interrupt = (value & 0x20) != 0;    // bit 5
	mode_1_vblank_interrupt = (value & 0x10) != 0; // bit 4
	mode_0_hblank_interrupt = (value & 0x08) != 0; // bit 3
	ly_flag = (value & 0x04) != 0;
	val = value;
}

uint8_t lcd_status::get() {
	// DEBUG_MSG("val: %u get: %u\n", val, (ly_interrupt << 6 | mode_2_oam_interrupt << 5 | mode_1_vblank_interrupt << 4
	// | mode_0_hblank_interrupt << 3 | ly_flag << 2 | mode));
	return (ly_interrupt << 6 | mode_2_oam_interrupt << 5 | mode_1_vblank_interrupt << 4 |
	        mode_0_hblank_interrupt << 3 | ly_flag << 2 | (val & 0x80) | (val & 0x3));
}

void lcd_dma::set(uint8_t value) {
	val = value;
	cycles = 640; // 160 M-cycles: 640 dots (1.4 lines) in normal speed, or 320 dots
	offset = 0;
}

void sprite_attributes::set(uint8_t value) {
	background = (value >> 7) & 1;
	y_flip = (value >> 6) & 1;
	x_flip = (value >> 5) & 1;
	palette = (value >> 4) & 1;
	bank = (value >> 3) & 1;
	cgb_pal = value & 0x07;
}

uint8_t sprite_attributes::get() {
	return (background << 7) | (y_flip << 6) | (x_flip << 5) | (palette << 4);
}