#include "Cpu.hpp"

void Cpu::event_handler() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			set_status(true);
			mmap.save_ram();
			DEBUG_MSG("breakpoint: %zu\n", debug_count);
		} else if ((e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) && !locked) {
			handle_input(e);
		}
		if (e.type == SDL_DROPFILE) {
			//TODO handle this properly
			char *dropped_filedir = e.drop.file;
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "File dropped on window", dropped_filedir,
			                         ppu.window());
			SDL_free(dropped_filedir);
		}
	}
	if (ppu.screen_ready() && !locked) {
		ppu.render_screen();
		ppu.screen_done();
	}
}