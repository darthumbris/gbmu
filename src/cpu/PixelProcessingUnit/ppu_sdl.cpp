#include "PixelProcessingUnit.hpp"
#include <SDL2/SDL_pixels.h>
#include <cstdint>

bool PixelProcessingUnit::init_window() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	} else {
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, 0, &data.window, &data.renderer);
		SDL_SetWindowTitle(data.window, "GBMU");
		if (data.window == NULL || data.renderer == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return false;
		} else {
			data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING,
			                                 SCREEN_WIDTH, SCREEN_HEIGHT);
			if (data.texture == NULL) {
				printf("texture could not be created! SDL_Error: %s\n", SDL_GetError());
				return false;
			}
		}
	}
	return true;
}

void PixelProcessingUnit::close() {
	SDL_DestroyTexture(data.texture);
	data.texture = NULL;
	SDL_DestroyRenderer(data.renderer);
	data.renderer = NULL;
	SDL_DestroyWindow(data.window);
	data.window = NULL;
	SDL_Quit();
}

//TODO add a toggle for color correction
//TODO add a toggle for matrix/rasterize
//TODO add a toggle for darkness filter
//TODO add a shader for: color correction, darkness filter, matrix/rasterize
void PixelProcessingUnit::render_screen() {
	if (!is_cgb) {
		for (int i = 0; i < SCREEN_PIXELS; i++) {
			rgb555_framebuffer[i] = GB_COLORS_ORIGNAL[mono_framebuffer[i]];
		}
	}
	SDL_RenderPresent(data.renderer);
	SDL_RenderClear(data.renderer);
	SDL_UpdateTexture(data.texture, NULL, rgb555_framebuffer, SCREEN_WIDTH * sizeof(uint16_t));
	SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
	SDL_RenderPresent(data.renderer);
	draw_screen = false;
}