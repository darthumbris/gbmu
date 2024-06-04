#include "PixelProcessingUnit.hpp"

PixelProcessingUnit::PixelProcessingUnit(/* args */)
{
    // mode = PPU_Modes::Horizontal_Blank;
    lcd_clock = 0;
    init_window();
    data.status = false;
    std::cout << "made ppu" << std::endl;
}

PixelProcessingUnit::~PixelProcessingUnit()
{
    // close();
}

void PixelProcessingUnit::tick(uint8_t cycle, MemoryMap &mmap)
{
    // std::cout << "lcd status: " << (uint16_t)mmap.get_lcd_status() << std::endl;
    if (mmap.get_lcd_enable()) {
        //TODO have a check for the dma (CGB only?) ?
        // std::cout << "ppu mode: " << (uint16_t)mmap.get_ppu_mode() << ", lcd_clock: " << lcd_clock << ", ly: "<< (uint16_t)mmap.get_lcd_line_y() << std::endl;
        // std::cout << "ly: " << (uint16_t)mmap.get_lcd_line_y() << std::endl;
        // std::cout << "cycle: " << (uint16_t)cycle / 4 << std::endl;
        lcd_clock += (uint16_t)cycle;
        // std::cout << "clock: " << lcd_clock / 4 << std::endl;

        //TODO get this working with the switch case stuff 

        if (lcd_clock >= 456) {
            lcd_clock -= 456;
            // mmap.
            mmap.increase_lcd_line_y_mod();
            uint8_t ly = mmap.get_lcd_line_y();
            if (mmap.get_ppu_mode() != PPU_Modes::Vertical_Blank && ly >= 144) {
                mmap.set_ppu_mode(PPU_Modes::Vertical_Blank);
                return;
            }
        }
        if (mmap.get_lcd_line_y() < 144) {
            if (lcd_clock <= 80) {
                if (mmap.get_ppu_mode() != PPU_Modes::OAM_Scan) {
                    mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
                    // println!("Mode Change: 2");
                }
            }
            else if (lcd_clock <= 252) {
                if (mmap.get_ppu_mode() != PPU_Modes::Pixel_Drawing) {
                    mmap.set_ppu_mode(PPU_Modes::Pixel_Drawing);
                    // println!("Mode Change: 3");
                }
            }
            else {
                if (mmap.get_ppu_mode() != PPU_Modes::Horizontal_Blank) {
                    mmap.set_ppu_mode(PPU_Modes::Horizontal_Blank);
                    // println!("Mode Change: 0");
                    render_scanline(mmap);
                }
            }
        }
        if (mmap.get_lcd_line_y() == 144) {
            render_screen();
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
    // std::cout << "bottom: " << (uint16_t)bottom << ", top: " << (uint16_t)top << std::endl;
    // std::cout << "tile index: " << (uint16_t)tile_index << std::endl;
    uint8_t pixel = ((top & (0x80 >> x)) == 0) ? 0x00 : 0b10;
    pixel |= ((bottom & (0x80 >> x)) == 0) ? 0x00 : 0b01;
    return pixel;
}

void PixelProcessingUnit::fill_pixels(std::deque<std::array<uint8_t, 2>> &pixels, uint16_t x, uint8_t n, uint8_t y, MemoryMap &mmap) {
    uint8_t vbank = mmap.vram_bank_select() ? 1 : 0; //TODO if this is correct
    uint16_t tile_map = mmap.get_background_tilemap() ? 0x1c00 : 0x1800;
    uint32_t line_offset = mmap.get_lcd_scrolling_x() >> 3;
    uint16_t map_number;
    if (mmap.get_bg_window_tiles()) {
            map_number = (64 - y / 8);
        } else {
            map_number = (y / 8);
        }
    // std::cout << "map number: " << map_number << std::endl;
    // tile_map += (((mmap.get_lcd_line_y() + mmap.get_lcd_scrolling_y()) & 255) >> 3) << 5;
    for (uint16_t p_x = x; p_x < (x + (uint16_t)n); p_x++) {
        if (mmap.get_window_enable() && mmap.get_lcd_window_x() == 0 + 7) {
            tile_map = (mmap.get_window_tilemap() ? 0x1C00 : 0x1800) + ((mmap.get_lcd_window_y() >> 3) << 5);
            // line_offset = 0;
        }
        uint8_t tile_index = mmap.get_tile_index(vbank, (p_x % 255) / 8, y / 8, tile_map);
        // std::cout << "map_offset: " << tile_map << " line_offset: " << line_offset << " tile_index: " << (uint16_t)mmap.vram[0][tile_map + line_offset] << std::endl;
        // if (!mmap.get_bg_window_tiles()) {
            //TODO check if this is needed?
            // tile_map = (384 - tile_map as u16) as u8;
        // }
        // if (tile_index)
        //     std::cout << "tile index: " << (uint16_t)tile_index << std::endl;
        // std::cout << "px: " << (uint16_t)p_x << ", y: " << (uint16_t)y << std::endl;
        // std::cout << "tile_map: " << (uint16_t)tile_map << std::endl;
        // std::cout << "xy : " << (uint16_t)((p_x % 255) / 8) << ", " << (uint16_t)(y / 8) << std::endl;
        // uint16_t x_pos = (uint16_t)((p_x % 255) / 8);
        // uint16_t y_pos = (uint16_t)(y / 8);
        // std::cout << "vram index: " << std::hex << 0x8000 + 0x1800 + (x_pos + y_pos * 32 + 32 * (uint16_t)tile_map * 32) << std::dec << std::endl;
        // std::cout << "vram size: " << mmap.vram[0].size() << std::endl;
        std::array<uint8_t, 2> pixel; 
        pixel[0] = get_pixel(tile_index, (p_x % 255) % 8, y % 8, mmap);
        pixel[1] = 0;
        pixels.push_back(pixel);
    }
}

void PixelProcessingUnit::set_pixel(uint32_t x, uint32_t y, uint32_t pixel, MemoryMap &mmap) {
    // std::cout << "set trying to set pixel: " << x <<", " << y<< std::endl;
    // std::cout << "pixels: " << data.screen->w << ", " << data.screen->h << ", pitch: " << data.screen->pitch << std::endl;
    // std::cout << "pixel offset: " << y * data.screen->pitch
    //                                          + x * data.screen->format->BytesPerPixel << std::endl;
    // std::cout << "x: " << x << ", y: " << y << std::endl;
    // uint32_t * const target_pixel = (uint32_t *) ((uint8_t *) data.screen->pixels
    //                                          + y * data.screen->pitch
    //                                          + x * data.screen->format->BytesPerPixel);
    // uint32_t * const target_pixel = (uint32_t *) ((uint8_t *) data.surface->pixels
    //                                          + y * data.surface->pitch
    //                                          + x * data.surface->format->BytesPerPixel);
    uint32_t color = 0x000000;
    uint32_t val = 0;
    switch (pixel) {
        case 0b00:
            val = mmap.get_background_palette_data() & 0b11;
            break;
        case 0b01:
            val = (mmap.get_background_palette_data() & 0b1100) >> 2;
            break;
        case 0b10:
            val = (mmap.get_background_palette_data() & 0b110000) >> 4;
            break;
        case 0b11:
            val = (mmap.get_background_palette_data() & 0b11000000) >> 6;
            break;
        default:
            break;
    }
    // if (val)
    // std::cout << "val: " << val << std::endl;
    switch (pixel) {
        case 0b00:
            color = 0xffe7cdff;
        break;
        case 0b01:
        color = 0xe4a39fff;
        break;
        case 0b10:
        color = 0x629098ff;
        break;
        case 0b11:
        color = 0x4c3457ff;
        break;
        default:
        color = 0x000000;
        break;
    }
    data.framebuffer[x + y * 160] = color;
    // SDL_SetRenderDrawColor(data.renderer, (uint8_t)(color << 24), (uint8_t)(color << 16), (uint8_t)(color << 8), 255);
    // SDL_RenderDrawPoint(data.renderer, x, y);
    // if (color != 0xFFFFFF)
    //     std::cout << "color: " << std::hex << color << std::dec << std::endl;
    // *target_pixel = color;
    // uint8_t* pixels = (uint8_t*)data.screen->pixels;
    // pixels[4 * (y * data.screen->pitch + x) + 0] = pixel;
    // pixels[4 * (y * data.screen->pitch + x) + 1] = pixel << 8;
    // pixels[4 * (y * data.screen->pitch + x) + 2] = pixel << 16;
    // pixels[4 * (y * data.screen->pitch + x) + 3] = pixel << 24;
}

void PixelProcessingUnit::render_scanline(MemoryMap &mmap) {
    std::deque<std::array<uint8_t, 2>> pixels;
    std::vector<Sprite> sprites;

    uint8_t start_y = (mmap.get_lcd_line_y() + mmap.get_lcd_scrolling_y()) % 255;
    // std::cout << "start_y: " << (uint16_t)start_y / 8 << std::endl;
    // std::cout << "ly: " << (uint16_t)mmap.get_lcd_line_y() << " scy: " << (uint16_t)mmap.get_lcd_scrolling_y() << std::endl;
    // std::cout << "window x: " << (uint16_t)mmap.get_lcd_window_x() << std::endl;
    // std::cout << "0x1c00 - 0x1800: " << 0x1c00 - 0x1800 << std::endl;
    uint32_t start_x = mmap.get_lcd_scrolling_x();
    fill_pixels(pixels, start_x, 16, start_y, mmap);
    // std::cout << "pixels filled: " << pixels.size() << std::endl;
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
    if (sprites.size())
    std::cout << "size: " << sprites.size() << std::endl;
    // std::cout << "pixels filled: " << pixels.size() << std::endl;
    for (uint32_t x = 0; x < 160; x++) {
        while (pixels.size() < 8) {
            start_x = (x + mmap.get_lcd_scrolling_x() + 8);
            fill_pixels(pixels, start_x, 8, start_y, mmap);
            // std::cout << "pixels filled: " << pixels.size() << std::endl;
        }
        uint8_t spr_num = 0;

        for (auto spr : sprites) {
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
            //           Not handling flipx or flipy
                        replacement.push_front(pixels[0]);
                        pixels.pop_front();
                    }
                    for (uint8_t y = 0; y < size; y++) {

                        std::array<uint8_t, 2> item = replacement[0];
                        replacement.pop_front();
                        if (item[1] != 0 || (item[0] != 0 && (spr.att_flags & 0b10000000) == 0)) {
                            pixels.push_front(item);
                        } else {
                            std::cout << "sx: " << spr.x_pos << " sy: " << spr.y_pos << " | x:" << x << " line:" << mmap.get_lcd_line_y() << std::endl;
                            // std::println("sx: {} sy: {} | x:{} line:{}", spr.x_pos, spr.y_pos, x, mmap.get_lcd_line_y());
                            // println!("sx: {} sy: {} | x:{} line:{}", sprite[1], sprite[0], x, line);
                            std::array<uint8_t, 2> sprite_pixel;
                            sprite_pixel[0] = get_pixel(spr.tile_index, x - spr.x_pos + y - 8, (mmap.get_lcd_line_y() + 8) - spr.y_pos, mmap);
                            sprite_pixel[1] = spr_num + 1;
                            pixels.push_front(sprite_pixel);
                        }
                    }
                }
            spr_num++;
        }
        set_pixel(x, static_cast<uint32_t>(mmap.get_lcd_line_y()), pixels[0][0], mmap);
        pixels.pop_front();
    }
    // std::cout << "pixels filled: " << pixels.size() << std::endl;
    // std::cout << "ly: " << (uint16_t)mmap.get_lcd_line_y() << std::endl;
    // std::cout << "trying to set pixel: " << 0 <<", " << static_cast<uint32_t>(mmap.get_lcd_line_y()) * 160<< std::endl;
    // data.screen->pixels[y * 160] = pixels[0][0];
    pixels.clear();
    sprites.clear();
}

void PixelProcessingUnit::render_screen() {
    // SDL_BlitSurface(data.screen, NULL, data.surface, NULL);
    // SDL_UpdateWindowSurface(data.window);
    //TODO check what are best functions to do this stuff?
    // SDL_SetRenderDrawColor(data.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    // SDL_RenderClear(data.renderer);
    SDL_RenderPresent(data.renderer);
    SDL_UpdateTexture(data.texture, NULL, data.framebuffer, 160 * sizeof(uint32_t));
    SDL_RenderClear(data.renderer);
    SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
    SDL_RenderPresent(data.renderer);
}

bool PixelProcessingUnit::init_window()
{   
    std::cout << "init window" << std::endl;
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
            // Get window surface
            // data.surface = SDL_GetWindowSurface(data.window);
            // data.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
            // if (data.screen == NULL || data.surface == NULL) {
            //     printf("Screen surface could not be created! SDL_Error: %s\n", SDL_GetError());
            //     success = false;
            // }
        }
    }
    for (int i = 0; i < 144*160; i++) {
        data.framebuffer[i] = 0;
    }
    std::cout << "success: " << success << std::endl;
    return success;
}

void PixelProcessingUnit::close() {
    // SDL_FreeSurface(data.screen);
    // data.screen = NULL;
    // SDL_FreeSurface(data.surface);
    // data.surface = NULL;
    SDL_DestroyTexture(data.texture);

    SDL_DestroyRenderer(data.renderer);

    // Destroy window
    SDL_DestroyWindow(data.window);
    data.window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}