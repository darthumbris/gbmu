#include "PixelProcessingUnit.hpp"

PixelProcessingUnit::PixelProcessingUnit()
{
    lcd_clock = 0;
    init_window();
    data.status = false;
    // std::cout << "made ppu" << std::endl;
}

PixelProcessingUnit::~PixelProcessingUnit()
{
}

void PixelProcessingUnit::tick(uint8_t cycle, MemoryMap *mmap)
{
    if (mmap->get_lcd_enable()) {
        // std::cout << "lcd on" << std::endl;
        //TODO have a check for the dma (CGB only?) ?
        lcd_clock += (uint16_t)cycle;
        //TODO get this working with the switch case stuff 

        if (lcd_clock >= 456) {
            lcd_clock -= 456;
            mmap->increase_lcd_line_y_mod();
            uint8_t ly = mmap->get_lcd_line_y();
            if (mmap->get_ppu_mode() != PPU_Modes::Vertical_Blank && ly >= 144) {
                mmap->set_ppu_mode(PPU_Modes::Vertical_Blank);
                return;
            }
        }
        if (mmap->get_lcd_line_y() < 144) {
            if (lcd_clock <= 80) {
                if (mmap->get_ppu_mode() != PPU_Modes::OAM_Scan) {
                    mmap->set_ppu_mode(PPU_Modes::OAM_Scan);
                }
            }
            else if (lcd_clock <= 252) {
                if (mmap->get_ppu_mode() != PPU_Modes::Pixel_Drawing) {
                    mmap->set_ppu_mode(PPU_Modes::Pixel_Drawing);
                }
            }
            else {
                if (mmap->get_ppu_mode() != PPU_Modes::Horizontal_Blank) {
                    mmap->set_ppu_mode(PPU_Modes::Horizontal_Blank);
                    render_scanline(mmap);
                }
            }
        }
        if (mmap->get_lcd_line_y() == 144) {
            render_screen(mmap);
        }

        // switch (mmap.get_ppu_mode())
        // {
        // case PPU_Modes::Horizontal_Blank:
        //     if (lcd_clock >= 204) {
        //         lcd_clock -= 204;
        //         // std::cout << "increasing value in 0xFF44 for hor blank" << std::endl;
        //         // mmap.increase_lcd_line_y();
        //         uint8_t ly = mmap.get_lcd_line_y();
        //         if (ly == 144) {
        //             mmap.set_ppu_mode(PPU_Modes::Vertical_Blank);
        //             handle_interrupt(true, mmap);
        //             render_screen(); //Renders the full screen when at the end of the 144 lines
        //         }
        //         else {
        //             mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
        //             handle_interrupt(true, mmap);
        //             if (mmap.get_window_enable() && mmap.get_lcd_window_y() == ly) {
        //                 mmap.set_bg_window_enable_priority(true);
        //             }
        //         }
        //     }
        //     break;
        // case PPU_Modes::Vertical_Blank:
        //     if (lcd_clock >= 456) {
        //         lcd_clock -= 456;
        //         // std::cout << "increasing value in 0xFF44 for vertical blank" << std::endl;
        //         mmap.increase_lcd_line_y();
        //         uint8_t ly = mmap.get_lcd_line_y();
        //         if (ly > 153) {
        //             mmap.reset_lcd_window_y();
        //             mmap.reset_lcd_line_y();
        //             mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
        //             handle_interrupt(true, mmap);
        //             mmap.set_bg_window_enable_priority(mmap.get_window_enable() && mmap.get_lcd_window_y() == ly);
        //         }
        //     }
        //     break;
        // case PPU_Modes::OAM_Scan:
        //     if (lcd_clock >= 80) {
        //         mmap.set_ppu_mode(PPU_Modes::Pixel_Drawing);
        //         lcd_clock -= 80;
        //     }
        //     break;
        // case PPU_Modes::Pixel_Drawing:
        //     // std::cout << "ppu mode: " << (uint16_t)mmap.get_ppu_mode() << ", lcd_clock: " << lcd_clock << ", ly: "<< (uint16_t)mmap.get_lcd_line_y() << std::endl;
        //     if (lcd_clock >= 172) {
        //         mmap.set_ppu_mode(PPU_Modes::Horizontal_Blank);
        //         lcd_clock -= 172;
        //         render_scanline(mmap);
        //         handle_interrupt(false, mmap);
        //     }
        //     break;

        // default:
        //     break;
        // }
    }
    else {
        // std::cout << "lcd off" << std::endl;
    }
}

void PixelProcessingUnit::handle_interrupt(bool val, MemoryMap *mmap) {
    if (mmap->get_ppu_mode() == PPU_Modes::Vertical_Blank) {

    }
    // if (mmap.get_ppu_mode() == PPU_Modes::Vertical_Blank) {

    // }
    // if (mmap.get_ppu_mode() != PPU_Modes::Pixel_Drawing) {

    // }
}

uint8_t PixelProcessingUnit::get_pixel(uint8_t tile_index, uint8_t x, uint8_t y, MemoryMap *mmap) {
    uint8_t vbank = mmap->vram_bank_select() ? 1 : 0;
    uint8_t *t = mmap->get_tile_data(vbank, tile_index);
    uint8_t bottom = t[(y * 2 + 1)];
    uint8_t top = t[(y * 2)];
    uint8_t pixel = ((top & (0x80 >> x)) == 0) ? 0x00 : 0b10;
    pixel |= ((bottom & (0x80 >> x)) == 0) ? 0x00 : 0b01;
    return pixel;
}

void PixelProcessingUnit::fill_pixels(std::deque<std::array<uint8_t, 2>> &pixels, uint16_t x, uint8_t n, uint8_t y, MemoryMap *mmap) {
    uint8_t vbank = mmap->vram_bank_select() ? 1 : 0; //TODO if this is correct
    uint16_t tile_map = mmap->get_background_tilemap() ? 0x1c00 : 0x1800;
    for (uint16_t p_x = x; p_x < (x + (uint16_t)n); p_x++) {
        if (mmap->get_window_enable() && mmap->get_lcd_window_x() == 0 + 7) {
            tile_map = (mmap->get_window_tilemap() ? 0x1C00 : 0x1800) + ((mmap->get_lcd_window_y() >> 3) << 5);
        }
        uint8_t tile_index = mmap->get_tile_index(vbank, (p_x % 255) / 8, y / 8, tile_map);
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

uint32_t get_color(uint8_t bg_palette, uint8_t palette_id) {
    uint8_t val = 0;
    switch (palette_id) {
        case 0b00:
            val = bg_palette & 0b11;
        break;
        case 0b01:
            val = (bg_palette & 0b1100) >> 2;
        break;
        case 0b10:
            val = (bg_palette & 0b110000) >> 4;
        break;
        case 0b11:
            val = (bg_palette & 0b11000000) >> 6;
        break;
    }
    switch (val) {
        case 0b00:    
            return 0xFFFFFFFF;
        case 0b01:
            return 0xAAAAAAFF;
        case 0b10:    
            return 0x555555FF;
        case 0b11:    
            return 0x00000000;
        default:       
            return 0x00000000;
    }
}

void PixelProcessingUnit::handle_sprites(std::vector<Sprite> sprites, std::deque<std::array<uint8_t, 2>> &pixels, uint32_t x, MemoryMap *mmap) {
    uint8_t ly = mmap->get_lcd_line_y();
    uint8_t spr_num = 0;
    for (auto spr : sprites) {
            std::cout << "sprites" << std::endl;
                if ((spr.x_pos < 8 && x == 0) || spr.x_pos == x + 8) {
                    uint8_t size = 8;
                    if (spr.x_pos < 8) {
                        size = spr.x_pos;
                    }
                    else if (spr.x_pos >= 160 - 8) {
                        size = 160 - spr.x_pos;
                    }
                    std::cout << "size: " << size << std::endl;
                    std::deque<std::array<uint8_t, 2>> replacement;
                    for (uint8_t y = 0; y < size; y++) {
                        //TODO handle flipx/flipy
                        replacement.push_front(pixels[0]);
                        pixels.pop_front();
                    }
                    for (uint8_t y = 0; y < size; y++) {

                        std::array<uint8_t, 2> item = replacement[0];
                        replacement.pop_front();
                        if (item[1] != 0 || (item[0] != 0 && (spr.att_flags & 0b10000000) == 0)) {
                            pixels.push_front(item);
                        } else {
                            std::cout << "sx: " << spr.x_pos << " sy: " << spr.y_pos << " | x:" << x << " line:" << ly << std::endl;
                            std::array<uint8_t, 2> sprite_pixel;
                            sprite_pixel[0] = get_pixel(spr.tile_index, x - spr.x_pos + y - 8, (ly + 8) - spr.y_pos, mmap);
                            sprite_pixel[1] = spr_num + 1;
                            pixels.push_front(sprite_pixel);
                        }
                    }
                }
            spr_num++;
        }
}

void PixelProcessingUnit::render_scanline(MemoryMap *mmap) {
    std::deque<std::array<uint8_t, 2>> pixels;
    std::vector<Sprite> sprites;
    uint8_t bg_palette = mmap->get_background_palette_data();

    uint8_t ly = mmap->get_lcd_line_y();
    uint8_t start_y = (ly + mmap->get_lcd_scrolling_y()) % 255;
    uint32_t start_x = mmap->get_lcd_scrolling_x();
    fill_pixels(pixels, start_x, 16, start_y, mmap);
    if (mmap->get_obj_enable()) {
        for (int i = 0; i < 40; i++) {
            //Sprite stuff
            Sprite spr = mmap->get_sprite(i);
            if (ly < spr.y_pos && ly >= spr.y_pos - 16) { //obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
                if (mmap->get_obj_size() || ly < spr.y_pos - 8) {
                    sprites.push_back(spr);
                    //TODO maybe have a check for sprites.size == 10
                }
            }
        }
    }
    if (sprites.size()) {
        std::cout << "drawing sprites: " << sprites.size() << std::endl;
    }
    for (uint32_t x = 0; x < 160; x++) {
        while (pixels.size() <= 8) {
            start_x = (x + mmap->get_lcd_scrolling_x() + 8);
            fill_pixels(pixels, start_x, 8, start_y, mmap);
        }
        handle_sprites(sprites, pixels, x, mmap);
        mmap->framebuffer[x + ly * 160] = get_color(bg_palette, pixels[0][0]);
        pixels.pop_front();
    }
}

void PixelProcessingUnit::render_screen(MemoryMap *mmap) {
    SDL_RenderPresent(data.renderer);
    SDL_UpdateTexture(data.texture, NULL, mmap->framebuffer, 160 * sizeof(uint32_t));
    SDL_RenderClear(data.renderer);
    SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
    SDL_RenderPresent(data.renderer);
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
            data.renderer = SDL_CreateRenderer(data.window, -1, SDL_RENDERER_ACCELERATED);
            if (data.renderer == NULL) {
                printf("renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                return false;
            }
            data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
            if (data.texture == NULL) {
                printf("texture could not be created! SDL_Error: %s\n", SDL_GetError());
                return false;
            }
        }
    }
    return success;
}

void PixelProcessingUnit::close() {
    SDL_DestroyTexture(data.texture);

    SDL_DestroyRenderer(data.renderer);

    // Destroy window
    SDL_DestroyWindow(data.window);
    data.window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}