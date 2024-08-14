#include "PixelProcessingUnit.hpp"

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
			data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
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

//TODO optimize this bit!!!
void PixelProcessingUnit::render_screen() {
	if (!is_cgb) {
		for (int i = 0; i < SCREEN_PIXELS; i++) {
			r5g6b5_framebuffer[i] = GB_COLORS_ORIGNAL[mono_framebuffer[i]];
		}
	}
	for (int i = 0; i < SCREEN_PIXELS; i++) {
		rgb_framebuffer[i].red = (((r5g6b5_framebuffer[i] >> 11) & 0x1F) * 255 + 15) / 31;
		rgb_framebuffer[i].green = (((r5g6b5_framebuffer[i] >> 5) & 0x3F) * 255 + 31) / 63;
		rgb_framebuffer[i].blue = ((r5g6b5_framebuffer[i] & 0x1F) * 255 + 15) / 31;

		//TODO this bit especially/ maybe have a toggle or something
		if (is_cgb) {
			uint8_t red = (uint8_t)(((rgb_framebuffer[i].red * 0.8125f) + (rgb_framebuffer[i].green * 0.125f) +
			                         (rgb_framebuffer[i].blue * 0.0625f)) *
			                        0.95f);
			uint8_t green = (uint8_t)(((rgb_framebuffer[i].green * 0.75f) + (rgb_framebuffer[i].blue * 0.25f)) * 0.95f);
			uint8_t blue = (uint8_t)((((rgb_framebuffer[i].red * 0.1875f) + (rgb_framebuffer[i].green * 0.125f) +
			                           (rgb_framebuffer[i].blue * 0.6875f))) *
			                         0.95f);

			rgb_framebuffer[i].red = red;
			rgb_framebuffer[i].green = green;
			rgb_framebuffer[i].blue = blue;
		}
	}
	SDL_RenderPresent(data.renderer);
	SDL_UpdateTexture(data.texture, NULL, rgb_framebuffer, SCREEN_WIDTH * sizeof(rgb_color));
	SDL_RenderClear(data.renderer);
	SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
	SDL_RenderPresent(data.renderer);
	draw_screen = false;
}