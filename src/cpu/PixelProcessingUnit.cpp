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
    // std::cout << "lcd status: " << (uint16_t)mmap.get_lcd_status() << std::endl;
    if (mmap.get_lcd_enable()) {
        //TODO have a check for the dma (CGB only?) ?
        // std::cout << "ppu mode: " << (uint16_t)mmap.get_ppu_mode() << ", lcd_clock: " << lcd_clock << ", ly: "<< (uint16_t)mmap.get_lcd_line_y() << std::endl;

        lcd_clock += (uint16_t)cycle;
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
            // std::cout << "ppu mode: " << (uint16_t)mmap.get_ppu_mode() << ", lcd_clock: " << lcd_clock << ", ly: "<< (uint16_t)mmap.get_lcd_line_y() << std::endl;
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

uint8_t PixelProcessingUnit::get_pixel(uint8_t tile_index, uint8_t x, uint8_t y, MemoryMap &mmap) {
    uint8_t vbank = mmap.vram_bank_select() ? 1 : 0; //TODO if this is correct
    uint8_t *t = mmap.get_tile_data(vbank, tile_index);
    uint8_t bottom = t[(y * 2 + 1)];
    uint8_t top = t[(y * 2)];
    uint8_t pixel = ((top & (0x80 >> x)) == 0) ? 0x00 : 0b10;
    pixel |= ((bottom & (0x80 >> x)) == 0) ? 0x00 : 0b01;
    return pixel;
}

void PixelProcessingUnit::fill_pixels(std::deque<std::array<uint8_t, 2>> &pixels, uint16_t x, uint8_t n, uint8_t y, MemoryMap &mmap) {
    uint8_t vbank = mmap.vram_bank_select() ? 1 : 0; //TODO if this is correct
    uint8_t tile_map = mmap.get_background_tilemap() ? 1 : 0;
    for (uint16_t p_x = x; p_x < (x + (uint16_t)n); p_x++) {
        uint8_t tile_index = mmap.get_tile_index(vbank, (p_x % 255) / 8, y / 8, tile_map);
        // if (!mmap.get_bg_window_tiles()) {
            //TODO check if this is needed?
            // tile_map = (384 - tile_map as u16) as u8;
        // }
        std::array<uint8_t, 2> pixel; 
        pixel[0] = get_pixel(tile_index, (p_x % 255) % 8, y % 8, mmap);
        pixel[1] = 0;
        pixels.push_back(pixel);
    }
}

void PixelProcessingUnit::render_scanline(MemoryMap &mmap) {
    std::deque<std::array<uint8_t, 2>> pixels;
    std::vector<Sprite> sprites;

    uint8_t y = (mmap.get_lcd_line_y() + mmap.get_lcd_scrolling_y()) % 255;
    uint8_t x = mmap.get_lcd_scrolling_x();
    fill_pixels(pixels, x, 16, y, mmap);
    std::cout << "pixels filled: " << pixels.size() << std::endl;
    if (mmap.get_obj_enable()) {
        uint8_t ly = mmap.get_lcd_line_y();
        for (int i = 0; i < 40; i++) {
            //Sprite stuff
            Sprite spr = mmap.get_sprite(i);
            if (ly < spr.y_pos && ly >= spr.y_pos - 16) { //obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
                if (mmap.get_obj_size() || ly < spr.y_pos - 8) {
                    sprites.push_back(spr);
                    //TODO maybe have a check for sprites.size == 10
                }
            }
        }
    }
        
}

void PixelProcessingUnit::render_screen() {
    // SDL_BlitSurface(data.screen, NULL, data.surface, NULL);
    // SDL_UpdateWindowSurface(data.window);
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