#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <fstream>

class Cpu;

constexpr int SCREEN_WIDTH = 160;
constexpr int SCREEN_HEIGHT = 144;
constexpr int SCREEN_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT;

constexpr uint16_t GB_COLORS_ORIGNAL[4] = {0x84A0, 0x4B40, 0x2AA0, 0x1200};
// constexpr uint16_t GB_COLORS_ORIGNAL[4] = {0x84A0, 0x4B40, 0x2AA0, 0x1200};
// constexpr uint16_t GB_COLORS_ORIGNAL[4] = {0x84A0, 0x4B40, 0x2AA0, 0x1200};
// constexpr uint16_t GB_COLORS_ORIGNAL[4] = {0x84A0, 0x4B40, 0x2AA0, 0x1200};

constexpr uint16_t RED_MASK = {0b0000'0000'0001'1111};
constexpr uint16_t GREEN_MASK = {0b0000'0011'1110'0000};
constexpr uint16_t BLUE_MASK = {0b0111'1100'0000'0000};
constexpr uint8_t AUTO_INC = {0b1000'0000};
constexpr uint8_t SPEC_INDEX = !AUTO_INC;
constexpr uint8_t PALETTE_SIZE = 64;

struct Sdl_Data {
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	bool status;
};

enum PPU_Modes { Horizontal_Blank, Vertical_Blank, OAM_Scan, Pixel_Drawing };

struct LCD_STATUS {
	bool ly_interrupt;            // bit 6
	bool mode_2_oam_interrupt;    // bit 5
	bool mode_1_vblank_interrupt; // bit 4
	bool mode_0_hblank_interrupt; // bit 3
	bool ly_flag;                 // bit2
	uint8_t mode;                 // bit 0-1
	uint8_t val;

	void set(uint8_t value);
};

struct LCD_CONTROL {
	bool lcd_enable;              // bit7
	bool window_tile_map_address; // bit6
	bool window_enable;           // bit5
	bool bg_window_tile_data;     // bit4
	bool bg_tile_map_address;     // bit3
	bool obj_size;                // bit 2
	bool obj_enable;              // bit 1
	bool bg_enable;               // bit 0
	uint8_t val;

	void set(uint8_t value);
};

struct LCD_DMA {
	uint8_t val;
	uint16_t cycles;
	uint8_t offset;
	void set(uint8_t value);
};

struct Sprite_Attributes {
	bool background;
	bool y_flip;
	bool x_flip;
	bool palette;
	void set(uint8_t value);
	uint8_t get();
};

enum HDMA_Register {
	HDMA_1,
	HDMA_2,
	HDMA_3,
	HDMA_4,
};

struct RGB_COLOR {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

// TODO instead of the att_flags use 4 bools: (background, y_flip, x_flip, palette)
struct Sprite {
	uint8_t y_pos;
	uint8_t x_pos;
	uint8_t tile_index;
	Sprite_Attributes attributes;
};

class PixelProcessingUnit {
private:
	uint32_t lcd_clock = 0;
	uint16_t lcd_clock_vblank = 0;
	Sdl_Data data;
	LCD_CONTROL ctrl;
	LCD_STATUS l_status;
	uint8_t scy = 0;
	uint8_t scx = 0;
	uint8_t ly = 0;
	uint8_t vblank_line = 0;
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

	uint8_t interrupt_signal = 0;

	// Horizontal blanking DMA (HDMA)
	uint16_t hdma_source = 0;
	uint16_t hdma_dest = 0;
	uint8_t hdma[5];
	bool hdma_enable = false;
	uint16_t hdma_bytes = 0;

	bool lcd_enabled = true;
	uint8_t window_line_active = 0;
	bool draw_screen = false; // similar to vblank check?
	bool drawn_scanline = false;
	uint8_t pixels_drawn = 0;
	uint8_t tile_drawn = 0;
	int16_t screen_off_cycles = 0;
	uint8_t hide_screen = 0;
	bool is_cgb;

	uint8_t mono_framebuffer[SCREEN_HEIGHT * SCREEN_WIDTH];
	uint16_t r5g6b6_framebuffer[SCREEN_HEIGHT * SCREEN_WIDTH];
	RGB_COLOR rgb_framebuffer[SCREEN_HEIGHT * SCREEN_WIDTH];

	int32_t sprite_cache_buffer[SCREEN_HEIGHT * SCREEN_WIDTH];
	uint8_t color_cache_buffer[SCREEN_HEIGHT * SCREEN_WIDTH];
	uint8_t oam[40][4]; // 0xFE00 - 0xFE9F 40 * 4 bytes(Byte 0: ypos, Byte1: Xpos, Byte2: tile_index, Byte3:
	                    // Attributes/flags)
	std::array<uint8_t, 8192> vram[2]{0};
	uint8_t tile_data[2][384][64];
	void set_tile_data(uint16_t addr);

	uint16_t cgb_bg_colors[8][4][2];
	uint16_t cgb_obj_colors[8][4][2];

	// TODO add shader for the rasterize effect
	// TODO add a way to change palettes (if gb game in CGB mode)

	Cpu *cpu;

	void handle_hblank(uint16_t &cycle);
	void handle_vblank(uint16_t &cycle);
	void handle_oam(uint16_t &cycle);
	void handle_pixel_drawing(uint16_t &cycle);

	void render_background(uint8_t line, uint8_t pixel);
	void render_window(uint8_t line);
	void render_sprites(uint8_t line);
	void render_scanline(uint8_t line);

	void dma_transfer(uint8_t cycle);

	void set_color_palette(bool background, uint8_t val);
	void update_palette_cgb(bool background, uint8_t val);

	void compare_ly();

	void disable_screen();
	void enable_screen();

public:
	PixelProcessingUnit(Cpu *cpu);
	~PixelProcessingUnit();

	void tick(uint16_t &cycle);
	bool init_window();
	void init_hdma();
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

	void switch_cgb_dma(uint8_t value);
	void set_hdma_register(HDMA_Register reg, uint8_t value);
	uint8_t get_hdma_register(uint8_t reg);

	uint8_t read_cgb_vram(uint16_t addr, bool force);

	inline uint8_t read_vram_remove(uint16_t addr, uint8_t bank) {
		return vram[bank][addr];
	}

	uint8_t perform_hdma();
	void perform_gdma(uint8_t value);

	inline uint8_t get_wram_bank_select() const {
		return wram_bank_select;
	}

	void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f);
};

#endif