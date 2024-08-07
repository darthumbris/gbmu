#include "Cpu.hpp"
#include "Interruptor.hpp"
#include <fstream>
#include <iostream>

void Cpu::serialize(const std::string &file) {
	std::ofstream f(file, std::ios::binary);
	if (!f.is_open()) {
		std::cerr << "Error: Failed to  open file for serialization Cpu." << std::endl;
		return;
	}
	f.write(reinterpret_cast<const char *>(&u8_registers), u8_registers.size());
	f.write(reinterpret_cast<const char *>(&sp), sizeof(sp));
	f.write(reinterpret_cast<const char *>(&pc), sizeof(pc));
	f.write(reinterpret_cast<const char *>(&halted), sizeof(halted));
	f.write(reinterpret_cast<const char *>(&m_cycle), sizeof(m_cycle));
	f.write(reinterpret_cast<const char *>(&t_cycle), sizeof(t_cycle));
	f.write(reinterpret_cast<const char *>(&opcode), sizeof(opcode));
	f.write(reinterpret_cast<const char *>(&debug_count), sizeof(debug_count));
	interruptor.serialize(f);
	mmap.serialize(f);
	ppu.serialize(f);
	f.close();
	std::cout << "done serializing" << std::endl;
}

void Cpu::deserialize(const std::string &file) {
	std::ifstream f(file, std::ios::binary);

	if (!f.is_open()) {
		std::cerr << "Error: Failed to  open file for deserialization Cpu." << std::endl;
		return;
	}
	f.read(reinterpret_cast<char *>(&u8_registers), sizeof(u8_registers));
	f.read(reinterpret_cast<char *>(&sp), sizeof(sp));
	f.read(reinterpret_cast<char *>(&pc), sizeof(pc));
	f.read(reinterpret_cast<char *>(&halted), sizeof(halted));
	f.read(reinterpret_cast<char *>(&m_cycle), sizeof(m_cycle));
	f.read(reinterpret_cast<char *>(&t_cycle), sizeof(t_cycle));
	f.read(reinterpret_cast<char *>(&opcode), sizeof(opcode));
	f.read(reinterpret_cast<char *>(&debug_count), sizeof(debug_count));
	interruptor.deserialize(f);
	mmap.deserialize(f);
	ppu.deserialize(f);
	f.close();
	std::cout << "done deserializing" << std::endl;
}

void Interruptor::serialize(std::ofstream &f) {
	f.write(reinterpret_cast<const char *>(&interrupt_enable_register), sizeof(interrupt_enable_register));
	f.write(reinterpret_cast<const char *>(&interrupt), sizeof(interrupt));
	f.write(reinterpret_cast<const char *>(&process_interrupts), sizeof(process_interrupts));
	
	f.write(reinterpret_cast<const char *>(&serial_transfer_data), sizeof(serial_transfer_data));
	f.write(reinterpret_cast<const char *>(&serial_transfer_control), sizeof(serial_transfer_control));
	
	f.write(reinterpret_cast<const char *>(&timer_divider), sizeof(timer_divider));
	f.write(reinterpret_cast<const char *>(&timer_counter), sizeof(timer_counter));
	f.write(reinterpret_cast<const char *>(&timer_modulo), sizeof(timer_modulo));
	f.write(reinterpret_cast<const char *>(&timer_enable), sizeof(timer_enable));
	f.write(reinterpret_cast<const char *>(&timer_clock_select), sizeof(timer_clock_select));

	f.write(reinterpret_cast<const char *>(&div_cycle), sizeof(div_cycle));
	f.write(reinterpret_cast<const char *>(&tima_cycle), sizeof(tima_cycle));
	
	f.write(reinterpret_cast<const char *>(&serial_cycle), sizeof(serial_cycle));
	f.write(reinterpret_cast<const char *>(&serial_count), sizeof(serial_count));
}
void Interruptor::deserialize(std::ifstream &f) {
	f.read(reinterpret_cast<char *>(&interrupt_enable_register), sizeof(interrupt_enable_register));
	f.read(reinterpret_cast<char *>(&interrupt), sizeof(interrupt));
	f.read(reinterpret_cast<char *>(&process_interrupts), sizeof(process_interrupts));
	
	f.read(reinterpret_cast<char *>(&serial_transfer_data), sizeof(serial_transfer_data));
	f.read(reinterpret_cast<char *>(&serial_transfer_control), sizeof(serial_transfer_control));
	
	f.read(reinterpret_cast<char *>(&timer_divider), sizeof(timer_divider));
	f.read(reinterpret_cast<char *>(&timer_counter), sizeof(timer_counter));
	f.read(reinterpret_cast<char *>(&timer_modulo), sizeof(timer_modulo));
	f.read(reinterpret_cast<char *>(&timer_enable), sizeof(timer_enable));
	f.read(reinterpret_cast<char *>(&timer_clock_select), sizeof(timer_clock_select));

	f.read(reinterpret_cast<char *>(&div_cycle), sizeof(div_cycle));
	f.read(reinterpret_cast<char *>(&tima_cycle), sizeof(tima_cycle));
	
	f.read(reinterpret_cast<char *>(&serial_cycle), sizeof(serial_cycle));
	f.read(reinterpret_cast<char *>(&serial_count), sizeof(serial_count));
}

void MemoryMap::serialize(std::ofstream &f) {
	f.write(reinterpret_cast<const char *>(&work_ram), sizeof(work_ram));
	f.write(reinterpret_cast<const char *>(&echo_ram), sizeof(echo_ram));
	f.write(reinterpret_cast<const char *>(&not_usable), sizeof(not_usable));
	f.write(reinterpret_cast<const char *>(&io_registers), sizeof(io_registers));
	f.write(reinterpret_cast<const char *>(&high_ram), sizeof(high_ram));
	f.write(reinterpret_cast<const char *>(&joypad), sizeof(joypad));
	f.write(reinterpret_cast<const char *>(&joypad_dpad), sizeof(joypad_dpad));
	f.write(reinterpret_cast<const char *>(&joypad_buttons), sizeof(joypad_buttons));
	f.write(reinterpret_cast<const char *>(&boot_rom_loaded), sizeof(boot_rom_loaded));
	f.write(reinterpret_cast<const char *>(&gb_boot_rom), sizeof(gb_boot_rom));
	f.write(reinterpret_cast<const char *>(&cgb_boot_rom), sizeof(cgb_boot_rom));
	std::cout << "done serializing mmap" << std::endl;
	rom->serialize(f);
}

void MemoryMap::deserialize(std::ifstream &f) {
	f.read(reinterpret_cast<char *>(&work_ram), sizeof(work_ram));
	f.read(reinterpret_cast<char *>(&echo_ram), sizeof(echo_ram));
	f.read(reinterpret_cast<char *>(&not_usable), sizeof(not_usable));
	f.read(reinterpret_cast<char *>(&io_registers), sizeof(io_registers));
	f.read(reinterpret_cast<char *>(&high_ram), sizeof(high_ram));
	f.read(reinterpret_cast<char *>(&joypad), sizeof(joypad));
	f.read(reinterpret_cast<char *>(&joypad_dpad), sizeof(joypad_dpad));
	f.read(reinterpret_cast<char *>(&joypad_buttons), sizeof(joypad_buttons));
	f.read(reinterpret_cast<char *>(&boot_rom_loaded), sizeof(boot_rom_loaded));
	f.read(reinterpret_cast<char *>(&gb_boot_rom), sizeof(gb_boot_rom));
	f.read(reinterpret_cast<char *>(&cgb_boot_rom), sizeof(cgb_boot_rom));
	std::cout << "done deserializing mmap" << std::endl;
	rom->deserialize(f);
}

void PixelProcessingUnit::serialize(std::ofstream &f) {
	f.write(reinterpret_cast<const char *>(&lcd_clock), sizeof(lcd_clock));
	f.write(reinterpret_cast<const char *>(&ctrl), sizeof(ctrl));
	f.write(reinterpret_cast<const char *>(&l_status), sizeof(l_status));
	f.write(reinterpret_cast<const char *>(&scy), sizeof(scy));
	f.write(reinterpret_cast<const char *>(&scx), sizeof(scx));
	f.write(reinterpret_cast<const char *>(&ly), sizeof(ly));
	f.write(reinterpret_cast<const char *>(&lyc), sizeof(lyc));
	f.write(reinterpret_cast<const char *>(&dma), sizeof(dma));
	f.write(reinterpret_cast<const char *>(&bg_palette), sizeof(bg_palette));
	f.write(reinterpret_cast<const char *>(&obj_palette_0), sizeof(obj_palette_0));
	f.write(reinterpret_cast<const char *>(&obj_palette_1), sizeof(obj_palette_1));
	f.write(reinterpret_cast<const char *>(&window_y), sizeof(window_y));
	f.write(reinterpret_cast<const char *>(&window_x), sizeof(window_x));
	f.write(reinterpret_cast<const char *>(&vbank_select), sizeof(vbank_select));
	f.write(reinterpret_cast<const char *>(&wram_bank_select), sizeof(wram_bank_select));
	f.write(reinterpret_cast<const char *>(&lcd_enabled), sizeof(lcd_enabled));
	f.write(reinterpret_cast<const char *>(&window_line_active), sizeof(window_line_active));
	f.write(reinterpret_cast<const char *>(&draw_screen), sizeof(draw_screen));
	f.write(reinterpret_cast<const char *>(&r5g6b6_framebuffer), sizeof(r5g6b6_framebuffer));
	f.write(reinterpret_cast<const char *>(&mono_framebuffer), sizeof(mono_framebuffer));
	f.write(reinterpret_cast<const char *>(&rgb_framebuffer), sizeof(rgb_framebuffer));
	f.write(reinterpret_cast<const char *>(&vram), sizeof(vram));
	std::cout << "done serializing ppu" << std::endl;
}

void PixelProcessingUnit::deserialize(std::ifstream &f) {
	f.read(reinterpret_cast<char *>(&lcd_clock), sizeof(lcd_clock));
	f.read(reinterpret_cast<char *>(&ctrl), sizeof(ctrl));
	f.read(reinterpret_cast<char *>(&l_status), sizeof(l_status));
	f.read(reinterpret_cast<char *>(&scy), sizeof(scy));
	f.read(reinterpret_cast<char *>(&scx), sizeof(scx));
	f.read(reinterpret_cast<char *>(&ly), sizeof(ly));
	f.read(reinterpret_cast<char *>(&lyc), sizeof(lyc));
	f.read(reinterpret_cast<char *>(&dma), sizeof(dma));
	f.read(reinterpret_cast<char *>(&bg_palette), sizeof(bg_palette));
	f.read(reinterpret_cast<char *>(&obj_palette_0), sizeof(obj_palette_0));
	f.read(reinterpret_cast<char *>(&obj_palette_1), sizeof(obj_palette_1));
	f.read(reinterpret_cast<char *>(&window_y), sizeof(window_y));
	f.read(reinterpret_cast<char *>(&window_x), sizeof(window_x));
	f.read(reinterpret_cast<char *>(&vbank_select), sizeof(vbank_select));
	f.read(reinterpret_cast<char *>(&wram_bank_select), sizeof(wram_bank_select));
	f.read(reinterpret_cast<char *>(&lcd_enabled), sizeof(lcd_enabled));
	f.read(reinterpret_cast<char *>(&window_line_active), sizeof(window_line_active));
	f.read(reinterpret_cast<char *>(&draw_screen), sizeof(draw_screen));
	f.read(reinterpret_cast<char *>(&r5g6b6_framebuffer), sizeof(r5g6b6_framebuffer));
	f.read(reinterpret_cast<char *>(&mono_framebuffer), sizeof(mono_framebuffer));
	f.read(reinterpret_cast<char *>(&rgb_framebuffer), sizeof(rgb_framebuffer));
	f.read(reinterpret_cast<char *>(&vram), sizeof(vram));
	std::cout << "done deserializing ppu" << std::endl;
}
