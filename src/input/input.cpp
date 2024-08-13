#include "Cpu.hpp"
#include <cstdio>

void Cpu::handle_input(SDL_Event &e) {
	SDL_Keymod modstate = SDL_GetModState();
	switch (e.type) {
	case SDL_KEYDOWN:
		get_mmap().handle_keydown(e.key.keysym.sym);
		switch (e.key.keysym.sym) {
		case SDLK_ESCAPE:
			set_status(true);
			DEBUG_MSG("breakpoint: %zu\n", debug_count);
			break;
		case SDLK_F1:
			if (modstate & KMOD_SHIFT) {
				deserialize(mmap.get_rom_name() + "_1.state");
			} else {
				serialize(mmap.get_rom_name() + "_1.state");
			}
			break;
		case SDLK_F2:
			if (modstate & KMOD_SHIFT) {
				deserialize(mmap.get_rom_name() + "_2.state");
			} else {
				serialize(mmap.get_rom_name() + "_2.state");
			}
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
	uint8_t old_key = read_u8(0xFF00);
	switch (key) {
	case SDLK_z: // A
		joypad_buttons &= ~(0x1);
		joypad_pressed &= (~(0x1 << 4));
		break;
	case SDLK_x: // B
		joypad_buttons &= ~(0x2);
		joypad_pressed &= (~(0x1 << 5));
		break;
	case SDLK_SPACE: // Select
		joypad_buttons &= ~(0x4);
		joypad_pressed &= (~(0x1 << 6));
		break;
	case SDLK_RETURN: // Start
		joypad_buttons &= ~(0x8);
		joypad_pressed &= (~(0x1 << 7));
		break;
	case SDLK_RIGHT: // >
		joypad_dpad &= ~(0x1);
		joypad_pressed &= (~(0x1 << 0));
		break;
	case SDLK_LEFT: // <
		joypad_dpad &= ~(0x2);
		joypad_pressed &= (~(0x1 << 1));
		break;
	case SDLK_UP: // ^
		joypad_dpad &= ~(0x4);
		joypad_pressed &= (~(0x1 << 2));
		break;
	case SDLK_DOWN: // v
		joypad_dpad &= ~(0x8);
		joypad_pressed &= (~(0x1 << 3));
		break;
	default:
		break;
	}
}

void MemoryMap::handle_keyup(SDL_Keycode key) {
	switch (key) {
	case SDLK_z: // A
		joypad_buttons |= (0x1);
		joypad_pressed |= ((0x1 << 4));
		break;
	case SDLK_x: // B
		joypad_buttons |= (0x2);
		joypad_pressed |= ((0x1 << 5));
		break;
	case SDLK_SPACE: // Select
		joypad_buttons |= (0x4);
		joypad_pressed |= ((0x1 << 6));
		break;
	case SDLK_RETURN: // Start
		joypad_buttons |= (0x8);
		joypad_pressed |= ((0x1 << 7));
		break;
	case SDLK_RIGHT: // >
		joypad_dpad |= (0x1);
		joypad_pressed |= ((0x1 << 0));
		break;
	case SDLK_LEFT: // <
		joypad_dpad |= (0x2);
		joypad_pressed |= ((0x1 << 1));
		break;
	case SDLK_UP: // ^
		joypad_dpad |= (0x4);
		joypad_pressed |= ((0x1 << 2));
		break;
	case SDLK_DOWN: // v
		joypad_dpad |= (0x8);
		joypad_pressed |= ((0x1 << 3));
		break;
	default:
		break;
	}
}