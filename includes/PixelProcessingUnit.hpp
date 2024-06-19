#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <cstdint>
#include <SDL2/SDL.h>
#include <deque>
#include <array>
#include <vector>
#include <iostream>

class Cpu;

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

struct Sdl_Data
{
    SDL_Window *window;
    SDL_Texture  *texture;
    SDL_Renderer *renderer;
    bool status;
};

enum PPU_Modes {
    Horizontal_Blank,
    Vertical_Blank,
    OAM_Scan,
    Pixel_Drawing
};

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

struct Sprite {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t att_flags;
};

class PixelProcessingUnit
{
private:
    uint16_t lcd_clock = 0;
    Sdl_Data data;

    LCD_CONTROL ctrl;
    LCD_STATUS  l_status;
    uint8_t scy = 0;
    uint8_t scx = 0;
    uint8_t ly = 1;
    uint8_t lyc = 0;
    LCD_DMA dma;
    uint8_t bg_palette = 0;
    uint8_t obj_palette_0 = 0;
    uint8_t obj_palette_1 = 0;
    uint8_t window_y = 0;
    uint8_t window_x = 0;
    uint8_t vbank_select = 0; //0xFF4F
    uint8_t wram_bank_select = 0; //0xFF70

    bool window_active;
    uint8_t window_line_active = 0;
    bool draw_screen = false;

    uint32_t framebuffer[SCREEN_HEIGHT*SCREEN_WIDTH];

    uint8_t oam[40][4]; // 0xFE00 - 0xFE9F 40 * 4 bytes(Byte 0: ypos, Byte1: Xpos, Byte2: tile_index, Byte3: Attributes/flags)
    std::array<uint8_t, 8192> vram[2]{0};
    uint8_t tile_data[2][384][64];

    uint32_t bg_colors[4];
    uint32_t obj_0_colors[4];
    uint32_t obj_1_colors[4];

    Cpu *cpu;

    void handle_interrupt(bool val);

    void render_scanline();
    void handle_sprites(std::vector<Sprite> sprites, uint32_t i, uint8_t tile_data_pos, uint32_t *framebuffer_ptr, size_t *spr_index);
    Sprite get_sprite(size_t index);

    void set_tile_data(uint16_t addr);
    void dma_transfer(uint8_t cycle);

public:
    PixelProcessingUnit(Cpu *cpu);
    ~PixelProcessingUnit();

    void tick(uint8_t cycle);
    bool init_window();
    void close();
    void render_screen();
    inline bool status() {return data.status;}
    inline void set_status(bool val) {data.status = val;}

    uint8_t read_u8_ppu(uint16_t addr);
    void write_u8_ppu(uint16_t addr, uint8_t val);

    uint8_t read_oam(uint16_t addr);
    void write_oam(uint16_t addr, uint8_t val);

    inline bool screen_ready() {return draw_screen;}
    inline void screen_done() {draw_screen = false;}
};

#endif