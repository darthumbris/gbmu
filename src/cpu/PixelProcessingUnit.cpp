#include "PixelProcessingUnit.hpp"

PixelProcessingUnit::PixelProcessingUnit(/* args */)
{
    // mode = PPU_Modes::Horizontal_Blank;
    lcd_clock = 0;
    data.status = false;
}

PixelProcessingUnit::~PixelProcessingUnit()
{
}

void PixelProcessingUnit::tick(uint8_t cycle, MemoryMap &mmap)
{
    if (mmap.get_lcd_enable()) {

        //TODO have a check for the dma (CGB only?) ?

        lcd_clock += cycle;
        switch (mmap.get_ppu_mode())
        {
        case PPU_Modes::Horizontal_Blank:
            if (lcd_clock >= 204) {
                lcd_clock -= 204;
                mmap.increase_lcd_line_y();
                uint8_t ly = mmap.get_lcd_line_y();
                if (ly == 144) {
                    mmap.set_ppu_mode(PPU_Modes::Vertical_Blank);
                    handle_interrupt(true, mmap);
                    render_screen(); //Renders the full screen when at the end of the 144 lines
                }
                else {
                    mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
                    handle_interrupt(true, mmap);
                    if (mmap.get_window_enable() && mmap.get_lcd_window_y() == ly) {
                        mmap.set_bg_window_enable_priority(true);
                    }
                }
            }
            break;
        case PPU_Modes::Vertical_Blank:
            if (lcd_clock >= 456) {
                lcd_clock -= 456;
                mmap.increase_lcd_line_y();
                uint8_t ly = mmap.get_lcd_line_y();
                if (ly > 153) {
                    mmap.reset_lcd_window_y();
                    mmap.reset_lcd_line_y();
                    mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
                    handle_interrupt(true, mmap);
                    mmap.set_bg_window_enable_priority(mmap.get_window_enable() && mmap.get_lcd_window_y() == ly);
                }
            }
            break;
        case PPU_Modes::OAM_Scan:
            if (lcd_clock >= 80) {
                mmap.set_ppu_mode(PPU_Modes::Pixel_Drawing);
                lcd_clock -= 80;
            }
            break;
        case PPU_Modes::Pixel_Drawing:
            if (lcd_clock >= 172) {
                mmap.set_ppu_mode(PPU_Modes::Horizontal_Blank);
                lcd_clock -= 172;
                render_scanline(mmap);
                handle_interrupt(false, mmap);
            }
            break;

        default:
            break;
        }
    }
}

void PixelProcessingUnit::handle_interrupt(bool val, MemoryMap &mmap) {
    if (mmap.get_ppu_mode() == PPU_Modes::Vertical_Blank) {

    }
    // if (mmap.get_ppu_mode() == PPU_Modes::Vertical_Blank) {

    // }
    // if (mmap.get_ppu_mode() != PPU_Modes::Pixel_Drawing) {

    // }
}

void PixelProcessingUnit::render_scanline(MemoryMap &mmap) {
    std::vector<Sprite> sprites;
    if (mmap.get_obj_enable()) {
        uint8_t ly = mmap.get_lcd_line_y();
        for (int i = 0; i < 40; i++) {
            //Sprite stuff
            Sprite spr = mmap.get_sprite(i);
            if (ly < spr.y_pos && ly >= spr.y_pos - 16) { //obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
                if (get_obj_size() || ly < s.y_pos - 8) {
                    sprites.push_back(spr);
                    //TODO maybe have a check for sprites.size == 10
                }
            }
        }
    }
        
}

void PixelProcessingUnit::render_screen() {
    SDL_BlitSurface(data.screen, NULL, data.surface, NULL);
    SDL_UpdateWindowSurface(data.window);
    //TODO check what are best functions to do this stuff?
}

bool PixelProcessingUnit::init_window()
{
    bool success = true;
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // Create window
        data.window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (data.window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            // Get window surface
            data.surface = SDL_GetWindowSurface(data.window);
        }
    }
    return success;
}

void PixelProcessingUnit::close() {
    SDL_FreeSurface(data.surface);
    data.surface = NULL;
    SDL_FreeSurface(data.screen);
    data.screen = NULL;

    // Destroy window
    SDL_DestroyWindow(data.window);
    data.window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}