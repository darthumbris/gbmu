#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <deque>
#include <iostream>
#include <vector>

class Cpu;

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

#define GB_COLOR0 0xFFFFFFFF
#define GB_COLOR1 0xAAAAAAFF
#define GB_COLOR2 0x555555FF
#define GB_COLOR3 0x00000000

const uint32_t GB_COLORS[4] = {GB_COLOR0, GB_COLOR1, GB_COLOR2, GB_COLOR3};

const uint16_t RED_MASK  = {0b0000'0000'0001'1111};
const uint16_t GREEN_MASK = {0b0000'0011'1110'0000};
const uint16_t BLUE_MASK = {0b0111'1100'0000'0000};
const uint8_t AUTO_INC = {0b1000'0000};
const uint8_t SPEC_INDEX = !AUTO_INC;
const uint8_t PALETTE_SIZE = 64;

struct Sdl_Data {
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	bool status;
};

enum PPU_Modes { Horizontal_Blank, Vertical_Blank, OAM_Scan, Pixel_Drawing };

struct LCD_STATUS {
	bool ly_interrupt;
	bool mode_2_oam_interrupt;
	bool mode_1_vblank_interrupt;
	bool mode_0_hblank_interrupt;
	bool ly_flag;
	uint8_t mode;
	uint8_t val;

	void set(uint8_t value);
};

struct LCD_CONTROL {
	bool lcd_enable;
	bool window_tile_map_address;
	bool window_enable;
	bool bg_window_tile_data;
	bool bg_tile_map_address;
	bool obj_size;
	bool obj_enable;
	bool bg_enable;
	uint8_t val;

	void set(uint8_t value);
};

struct LCD_DMA {
	uint8_t val;
	uint16_t cycles;
	uint8_t offset;
	void set(uint8_t value);
};

//TODO instead of the att_flags use 4 bools: (background, y_flip, x_flip, palette)
struct Sprite {
	uint8_t y_pos;
	uint8_t x_pos;
	uint8_t tile_index;
	uint8_t att_flags;
};

class PixelProcessingUnit {
private:
	uint16_t lcd_clock = 0;
	Sdl_Data data;
	LCD_CONTROL ctrl;
	LCD_STATUS l_status;
	uint8_t scy = 0;
	uint8_t scx = 0;
	uint8_t ly = 1;
	uint8_t lyc = 0;
	LCD_DMA dma;
	uint8_t bg_palette = 0;      // 0xFF47 (Non_CGB_Mode)
	uint8_t obj_palette_0 = 0;   // 0xFF48 (Non_CGB_Mode)
	uint8_t obj_palette_1 = 0;   // 0xFF49 (Non_CGB_Mode)
	uint8_t bg_palette_cgb = 0;  // 0xFF68 (CGB_ONLY) bg palette spec and index
	uint8_t bg_color_cgb = 0;    // 0xFF69 (CGB_ONLY) bg palette color data (rgb)
	uint8_t obj_palette_cgb = 0; // 0xFF6A (CGB_ONLY) obj palette spec and index
	uint8_t obj_color_cgb = 0;   // 0xFF6B (CGB_ONLY) obj palette color data (rgb)
	uint8_t window_y = 0;
	uint8_t window_x = 0;
	uint8_t vbank_select = 0;     // 0xFF4F
	uint8_t wram_bank_select = 0; // 0xFF70

	bool window_active = false;
	uint8_t window_line_active = 0;
	bool draw_screen = false;
	bool cgb_colors;

	uint32_t framebuffer[SCREEN_HEIGHT * SCREEN_WIDTH];

	uint8_t oam[40][4]; // 0xFE00 - 0xFE9F 40 * 4 bytes(Byte 0: ypos, Byte1: Xpos, Byte2: tile_index, Byte3:
	                    // Attributes/flags)
	std::array<uint8_t, 8192> vram[2]{0};
	uint8_t tile_data[2][384][64];

	uint32_t bg_colors[4];
	uint8_t cgb_bg_colors[64];
	uint16_t cgb_bg_colors_other[8][4][2];
	uint32_t cgb_bg_colors_other_32[8][4];
	uint32_t obj_0_colors[4];
	uint32_t obj_1_colors[4];

	// TODO add CGB colors and make constants of some of the colors
	// TODO add shader for the rasterize effect
	// TODO add a way to change palettes (if gb game in CGB mode)

	Cpu *cpu;

	void handle_interrupt(bool val);

	void render_scanline();
	void handle_sprites(std::vector<Sprite> sprites, uint32_t i, uint8_t tile_data_pos, uint32_t *framebuffer_ptr,
	                    size_t *spr_index);
	Sprite get_sprite(size_t index);

	void set_tile_data(uint16_t addr);
	void dma_transfer(uint8_t cycle);
	uint32_t get_cgb_color(uint8_t value1, uint8_t value2);
	void set_cgb_bg_palette(uint8_t val);
	void update_palette_cgb(uint8_t val);

public:
	PixelProcessingUnit(Cpu *cpu);
	~PixelProcessingUnit();

	void tick(uint8_t cycle);
	bool init_window();
	void close();
	void render_screen();
	inline bool status() {
		return data.status;
	}
	inline void set_status(bool val) {
		data.status = val;
	}

	uint8_t read_u8_ppu(uint16_t addr);
	void write_u8_ppu(uint16_t addr, uint8_t val);

	uint8_t read_oam(uint16_t addr);
	void write_oam(uint16_t addr, uint8_t val);

	inline bool screen_ready() {
		return draw_screen;
	}
	inline void screen_done() {
		draw_screen = false;
	}
	inline void set_cpu(Cpu *cpu) {
		cpu = cpu;
	}
	void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f);
};

#endif