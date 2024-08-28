#include "Cpu.hpp"
#include <SDL2/SDL_keycode.h>
#include <cstdio>

// TODO add a way to pause the gamestate

void Cpu::handle_input(SDL_Event &e) {
	SDL_Keymod modstate = SDL_GetModState();
	switch (e.type) {
	case SDL_KEYDOWN:
		get_mmap().handle_keydown(e.key.keysym.sym);
		switch (e.key.keysym.sym) {
		case SDLK_ESCAPE:
			set_status(true);
			mmap.save_ram();
			DEBUG_MSG("breakpoint: %zu\n", debug_count);
			break;
		case SDLK_p:
			modstate &KMOD_SHIFT ? ppu.decrease_palette() : ppu.increase_palette();
			break;
		case SDLK_F1:
			modstate &KMOD_SHIFT ? deserialize(mmap.get_rom_name() + "_1.state")
			                     : serialize(mmap.get_rom_name() + "_1.state");
			break;
		case SDLK_F2:
			modstate &KMOD_SHIFT ? deserialize(mmap.get_rom_name() + "_2.state")
			                     : serialize(mmap.get_rom_name() + "_2.state");
			break;
		case SDLK_m:
			ppu.toggle_matrix();
			break;
		case SDLK_c:
			ppu.toggle_color_correction();
			break;
		case SDLK_d:
			ppu.toggle_darkening();
			break;
		case SDLK_MINUS:
			apu.set_volume(0.1);
			break;
		case SDLK_PLUS:
			apu.set_volume(1.0);
			break;
		default:
			break;
		}
		break;
	case SDL_KEYUP:
		get_mmap().handle_keyup(e.key.keysym.sym);
		break;
	default:
		break;
	}
}

void MemoryMap::handle_keydown(SDL_Keycode key) {
	switch (key) {
	case SDLK_z: // A
		joypad_pressed &= (~(0x1 << 4));
		break;
	case SDLK_x: // B
		joypad_pressed &= (~(0x1 << 5));
		break;
	case SDLK_SPACE: // Select
		joypad_pressed &= (~(0x1 << 6));
		break;
	case SDLK_RETURN: // Start
		joypad_pressed &= (~(0x1 << 7));
		break;
	case SDLK_RIGHT: // >
		joypad_pressed &= (~(0x1 << 0));
		break;
	case SDLK_LEFT: // <
		joypad_pressed &= (~(0x1 << 1));
		break;
	case SDLK_UP: // ^
		joypad_pressed &= (~(0x1 << 2));
		break;
	case SDLK_DOWN: // v
		joypad_pressed &= (~(0x1 << 3));
		break;
	default:
		break;
	}
}

void MemoryMap::handle_keyup(SDL_Keycode key) {
	switch (key) {
	case SDLK_z: // A
		joypad_pressed |= ((0x1 << 4));
		break;
	case SDLK_x: // B
		joypad_pressed |= ((0x1 << 5));
		break;
	case SDLK_SPACE: // Select
		joypad_pressed |= ((0x1 << 6));
		break;
	case SDLK_RETURN: // Start
		joypad_pressed |= ((0x1 << 7));
		break;
	case SDLK_RIGHT: // >
		joypad_pressed |= ((0x1 << 0));
		break;
	case SDLK_LEFT: // <
		joypad_pressed |= ((0x1 << 1));
		break;
	case SDLK_UP: // ^
		joypad_pressed |= ((0x1 << 2));
		break;
	case SDLK_DOWN: // v
		joypad_pressed |= ((0x1 << 3));
		break;
	default:
		break;
	}
}