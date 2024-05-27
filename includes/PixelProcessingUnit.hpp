#ifndef PIXELPROCESSINGUNIT_HPP
#define PIXELPROCESSINGUNIT_HPP

#include <cstdint>
#include <SDL2/SDL.h>
#include "MemoryMap.hpp"

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

struct Sdl_Data
{
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Surface *screen;
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
    // PPU_Modes mode;
    // uint8_t lcd_control; //0xFF40 (bits: 7:lcd/ppu enable, 6:window tilemap, 5: window enable, 4: bg/window tiles, 3: bg tilemap, 2: objsize, 1: obj enable, 0: bg/window enable/priority)
    // uint8_t stat;        //0xFF41 (bits: 6:LYC select, 5:mode2 select, 4:mode1 select, 3:mode0 select,2:LYC==LY,(1,0): ppu_mode)
    // uint8_t ly;          //0xFF44 (Line y)
    // uint8_t lcy;         //0xFF45 (Line y compare)
    // uint8_t scy;         //background viewport y pos
    // uint8_t scx;         //background viewport x pos
    // uint8_t wy;          //window y pos
    // uint8_t wx;          //window x pos
    Sdl_Data data;

    void handle_interrupt(bool val, MemoryMap &mmap);

    void render_scanline(MemoryMap &mmap);

public:
    PixelProcessingUnit(/* args */);
    ~PixelProcessingUnit();

    void tick(uint8_t interrupt, MemoryMap &mmap);
    bool init_window();
    void close();
    void render_screen();
    inline bool status() {return data.status;}
    inline void set_status(bool val) {data.status = val;}
};

#endif