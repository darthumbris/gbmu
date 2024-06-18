#include "Cpu.hpp"

void Cpu::handle_input(SDL_Event &e)
{
    switch (e.type) {
        case SDL_KEYDOWN: 
            get_mmap().handle_keydown(e.key.keysym.sym);
            break;
        case SDL_KEYUP: 
            get_mmap().handle_keyup(e.key.keysym.sym);
            break;
        default:
            break;
    }
    switch (e.key.keysym.sym)
    {
    case SDLK_ESCAPE:
        set_status(true);
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
            break;
        case SDLK_x: // B
            joypad_buttons &= ~(0x2);
            break;
        case SDLK_SPACE: // Select
            joypad_buttons &= ~(0x4);
            break;
        case SDLK_RETURN: //Start
            joypad_buttons &= ~(0x8);
            break;
        case SDLK_RIGHT: // >
            joypad_dpad &= ~(0x1);
            break;
        case SDLK_LEFT: // <
            joypad_dpad &= ~(0x2);
            break;
        case SDLK_UP: // ^
            joypad_dpad &= ~(0x4);
            break;
        case SDLK_DOWN: // v
            joypad_dpad &= ~(0x8);
            break;
        default: 
            break;
    }
    
    if (old_key != read_u8(0xFF00)) {
       cpu->set_interrupt(InterruptType::Joypad); 
    }
}

void MemoryMap::handle_keyup(SDL_Keycode key) {
    switch (key) {
        case SDLK_z: // A
            joypad_buttons |= (0x1);
            break;
        case SDLK_x: // B
            joypad_buttons |= (0x2);
            break;
        case SDLK_SPACE: // Select
            joypad_buttons |= (0x4);
            break;
        case SDLK_RETURN: //Start
            joypad_buttons |= (0x8);
            break;
        case SDLK_RIGHT: // >
            joypad_dpad |= (0x1);
            break;
        case SDLK_LEFT: // <
            joypad_dpad |= (0x2);
            break;
        case SDLK_UP: // ^
            joypad_dpad |= (0x4);
            break;
        case SDLK_DOWN: // v
            joypad_dpad |= (0x8);
            break;
        default: 
            break;
    }
}