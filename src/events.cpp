#include "Cpu.hpp"
#include "MemoryMap.hpp"
#include "debug.hpp"

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
			char *dropped_filedir = e.drop.file;
			DEBUG_MSG("loading rom: %s\n", dropped_filedir);
			rom_path = dropped_filedir;
			load_options.path = dropped_filedir;
			mmap.reset_file(load_options);
			reset();
			SDL_free(dropped_filedir);
		}
	}
	if (ppu.screen_ready() && !locked) {
		ppu.render_screen();
		ppu.screen_done();
	}
}