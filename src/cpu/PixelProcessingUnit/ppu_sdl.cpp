#include "PixelProcessingUnit.hpp"
// #include <SDL2/SDL.h>
// #include <glew.h>
// #include <SDL2/SDL_pixels.h>
// #include <cstdint>
// #include <SDL2/SDL_opengl.h>
// #include <glu.h>

// bright color, higlight color, shadow color , unlit color
constexpr uint16_t GB_COLORS_ORIGNAL[4] = {0xC240, 0xA5A0, 0x9540, 0x8900};
// constexpr uint16_t GB_COLORS_TEST[4] = {0XFC43, 0XEC64, 0XD065, 0X8C63};
constexpr uint16_t GB_COLORS_VIRTUABOY[4] = {0XFC43, 0XEC64, 0XD065, 0X8C63};
constexpr uint16_t GB_COLORS_LIGHT[4] = {0XE75D, 0XCF3D, 0XB2B7, 0X8002};
constexpr uint16_t GB_COLORS_BW[4] = {0XF7BD, 0XDAD6, 0XCA52, 0X8C63};

// void check_color_rgb555(uint8_t red, uint8_t green, uint8_t blue) {
// 	int multiplier = 31;
//     int shift = 10;

// 	uint16_t color0;
// 	color0 = (((red * 31) / 255) << shift ) | (((green * multiplier) / 255) << 5 ) | ((blue * 31) / 255);
// 	color0 |= 0x8000;
// 	printf("color: %#06X r: %u g: %u b: %u\n", color0, red, green, blue);
// }

bool PixelProcessingUnit::init_window() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	} else {
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, 0, &data.window, &data.renderer);
		SDL_SetWindowTitle(data.window, "GBMU");
		if (data.window == nullptr || data.renderer == nullptr) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return false;
		} else {
			data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING,
			                                 SCREEN_WIDTH, SCREEN_HEIGHT);
			if (data.texture == nullptr) {
				printf("texture could not be created! SDL_Error: %s\n", SDL_GetError());
				return false;
			}
		}
	}
	return true;
}

void PixelProcessingUnit::close() {
	SDL_DestroyTexture(data.texture);
	data.texture = nullptr;
	SDL_DestroyRenderer(data.renderer);
	data.renderer = nullptr;
	SDL_DestroyWindow(data.window);
	data.window = nullptr;
	SDL_Quit();
}

// TODO add a toggle for color correction
// TODO add a toggle for matrix/rasterize
// TODO add a toggle for darkness filter
// TODO add a shader for: color correction, darkness filter, matrix/rasterize
void PixelProcessingUnit::render_screen() {
	const uint16_t *palette_used;
	switch (current_palette) {
	case 0:
		palette_used = GB_COLORS_ORIGNAL;
		break;
	case 1:
		palette_used = GB_COLORS_VIRTUABOY;
		break;
	case 2:
		palette_used = GB_COLORS_LIGHT;
		break;
	default:
		palette_used = GB_COLORS_BW;
		break;
	}
	if (!is_cgb) {
		for (int i = 0; i < SCREEN_PIXELS; i++) {
			rgb555_framebuffer[i] = palette_used[mono_framebuffer[i]];
		}
	}
	SDL_RenderPresent(data.renderer);
	SDL_RenderClear(data.renderer);
	SDL_UpdateTexture(data.texture, nullptr, rgb555_framebuffer, SCREEN_WIDTH * sizeof(uint16_t));
	SDL_RenderCopy(data.renderer, data.texture, nullptr, nullptr);
	SDL_RenderPresent(data.renderer);
	draw_screen = false;
}

void PixelProcessingUnit::increase_palette() {
	current_palette += 1;
	if (current_palette >= MAX_PALETTES) {
		current_palette = 0;
	}
}
void PixelProcessingUnit::decrease_palette() {
	if (current_palette == 0) {
		current_palette = MAX_PALETTES - 1;
	} else {
		current_palette -= 1;
	}
}