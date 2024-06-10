#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <cstdint>
#include <SDL2/SDL.h>
#include "MemoryMap.hpp"
#include <deque>
#include <array>
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

class PixelProcessingUnit
{
private:
    uint16_t lcd_clock;
    Sdl_Data data;

    void handle_interrupt(bool val, MemoryMap *mmap);

    void render_scanline(MemoryMap *mmap);
    uint8_t get_pixel(uint8_t tile_index, uint8_t x, uint8_t y, MemoryMap *mmap);
    void fill_pixels(std::deque<std::array<uint8_t, 2>> &pixels, uint16_t x, uint8_t n, uint8_t y, MemoryMap *mmap);
    void set_pixel(uint32_t x, uint32_t y, uint8_t pixel, MemoryMap *mmap);
    void handle_sprites(std::vector<Sprite> sprites, std::deque<std::array<uint8_t, 2>> &pixels, uint32_t x, MemoryMap *mmap);

public:
    PixelProcessingUnit();
    ~PixelProcessingUnit();

    void tick(uint8_t interrupt, MemoryMap *mmap);
    bool init_window();
    void close();
    void render_screen(MemoryMap *mmap);
    inline bool status() {return data.status;}
    inline void set_status(bool val) {data.status = val;}
};

#endif