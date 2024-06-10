#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <cstdint>
#include <SDL2/SDL.h>
// #include "MemoryMap.hpp"
#include <deque>
#include <array>
#include <vector>
#include <iostream>

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

struct Sprite {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t att_flags;
};

//TODO prob make all the lcd stuff bools and u8's?

class PixelProcessingUnit
{
private:
    uint16_t lcd_clock;
    Sdl_Data data;

    LCD_CONTROL ctrl;
    LCD_STATUS  l_status;
    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t dma;
    uint8_t bg_palette;
    uint8_t obj_palette_0;
    uint8_t obj_palette_1;
    uint8_t window_y;
    uint8_t window_x;
    uint8_t vbank_select; //0xFF4F
    uint8_t wram_bank_select; //0xFF70

    uint32_t framebuffer[144*160];

    uint8_t oam[40][4];
    // std::array<uint8_t, 160> oam{0};         // 0xFE00 - 0xFE9F   // 40 * 4 bytes(Byte 0: ypos, Byte1: Xpos, Byte2: tile_index, Byte3: Attributes/flags)
    uint8_t tile_data_0[384][16];
    uint8_t tile_map_0[64][32];
    // std::array<std::array<uint8_t, 16>, 384> tile_data_0;
    // std::array<std::array<uint8_t, 32>, 64> tile_map_0;
    // std::array<std::array<uint8_t, 16>, 384> tile_data_1;
    // std::array<std::array<uint8_t, 32>, 64> tile_map_1;
    uint8_t tile_data_1[384][16];
    uint8_t tile_map_1[64][32];

    uint8_t bg_colors[4];

    void handle_interrupt(bool val);

    void render_scanline();
    uint8_t get_pixel(uint8_t tile_index, uint8_t x, uint8_t y);
    void fill_pixels(std::deque<std::array<uint8_t, 2>> &pixels, uint16_t x, uint8_t n, uint8_t y, uint16_t map_number);
    void set_pixel(uint32_t x, uint32_t y, uint8_t pixel);
    void handle_sprites(std::vector<Sprite> sprites, std::deque<std::array<uint8_t, 2>> &pixels, uint32_t x);
    Sprite get_sprite(size_t index);

public:
    PixelProcessingUnit();
    ~PixelProcessingUnit();

    void tick(uint8_t interrupt);
    bool init_window();
    void close();
    void render_screen();
    inline bool status() {return data.status;}
    inline void set_status(bool val) {data.status = val;}

    uint8_t read_u8_ppu(uint16_t addr);
    void write_u8_ppu(uint16_t addr, uint8_t val);

    uint8_t read_oam(uint16_t addr);
    void write_oam(uint16_t addr, uint8_t val);
};

#endif