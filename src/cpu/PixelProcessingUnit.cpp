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
                    render_screen();
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
                if (ly == 153) {
                    //window line = 0; ?
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
    if (mmap.get_obj_enable()) {
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