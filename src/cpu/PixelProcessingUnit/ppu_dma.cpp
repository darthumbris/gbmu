#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"

void PixelProcessingUnit::init_hdma() {
	if (is_cgb) {
		for (int i = 0; i < 5; i++) {
			hdma[i] = cpu->get_mmap().read_io_registers(0xFF51 + i);
		}

		uint8_t hdma1 = hdma[0];
		uint8_t hdma2 = hdma[1];
		uint8_t hdma3 = hdma[2];
		uint8_t hdma4 = hdma[3];

		if (hdma1 > 0x7f && hdma1 < 0xa0)
			hdma1 = 0;

		hdma_source = (hdma1 << 8) | (hdma2 & 0xF0);
		hdma_dest = ((hdma3 & 0x1F) << 8) | (hdma4 & 0xF0);
		hdma_dest |= 0x8000;
	}
}

void PixelProcessingUnit::dma_transfer(uint8_t cycle) {

	if (is_cgb) {
		uint16_t src = (dma.val << 8);
		if (src < 0xE000) {
			if (src >= 0x8000 && src < 0xA000) {
				for (uint16_t i = 0; i < 0xA0; i++) {
					write_oam(0xFE00 + i, vram[vbank_select][(src + i) - 0x8000]);
				}
			} else {
				for (uint16_t i = 0; i < 0xA0; i++) {
					write_oam(0xFE00 + i, cpu->get_mmap().read_u8(src + i));
				}
			}
		}
	} else {
		uint16_t src = (cycle << 8);
		if (src >= 0x8000 && src <= 0xE000) {
			for (uint16_t v = 0; v < 0xA0; v++) {
				uint8_t value = cpu->get_mmap().read_u8(src + v);
				write_oam(0xFE00 + v, value);
			}
		}
	}
}

void PixelProcessingUnit::switch_cgb_dma(uint8_t value) {
	hdma_bytes = 16 + ((value & 0x7f) * 16);

	if (hdma_enable) {
		if (value & mask7) {
			hdma[4] = value & 0x7F;
		} else {
			hdma[4] = 0xFF;
			hdma_enable = false;
		}
	} else {
		if (value & mask7) {
			hdma_enable = true;
			hdma[4] = value & 0x7F;
			if (l_status.mode == PPU_Modes::Horizontal_Blank) {
				cpu->set_cycle_16(perform_hdma());
			}
		} else {
			perform_gdma(value);
		}
	}
}

void PixelProcessingUnit::set_hdma_register(HDMA_Register reg, uint8_t value) {
	switch (reg) {
	case HDMA_1:
		if (value > 0x7f && value < 0xa0) {
			value = 0;
		}
		hdma_source = (value << 8) | (hdma_source & 0xF0);
		break;
	case HDMA_2:
		value &= 0xF0;
		hdma_source = (hdma_source & 0xFF00) | value;
		break;
	case HDMA_3:
		value &= 0x1F;
		hdma_dest = (value << 8) | (hdma_dest & 0xF0);
		hdma_dest |= 0x8000;
		break;
	case HDMA_4:
		value &= 0xF0;
		hdma_dest = (hdma_dest & 0x1F00) | value;
		hdma_dest |= 0x8000;
		break;
	}
}

uint8_t PixelProcessingUnit::get_hdma_register(uint8_t reg) {
	return hdma[reg - 1];
}

// This is the gameboy color Horizontal Blanking DMA
uint16_t PixelProcessingUnit::perform_hdma() {
	uint16_t source = hdma_source & 0xFFF0;
	uint16_t destination = (hdma_dest & 0x1FF0) | 0x8000;

	for (int i = 0; i < 0x10; i++) {
		write_u8_ppu(destination + i, cpu->get_mmap().read_u8(source + i));
	}

	hdma_dest += 0x10;
	if (hdma_dest == 0xA000) {
		hdma_dest = 0x8000;
	}

	hdma_source += 0x10;
	if (hdma_source == 0x8000) {
		hdma_source = 0xA000;
	}

	hdma[1] = hdma_source & 0xFF;
	hdma[0] = hdma_source >> 8;
	hdma[3] = hdma_dest & 0xFF;
	hdma[2] = hdma_dest >> 8;
	hdma_bytes -= 0x10;
	hdma[4]--;

	if (hdma[4] == 0xFF) {
		hdma_enable = false;
	}

	return (cpu->get_cgb_speed() ? 17 : 9) * 4;
}

// This is the gameboy color general purpose DMA
void PixelProcessingUnit::perform_gdma(uint8_t value) {
	uint16_t source = hdma_source & 0xFFF0;
	uint16_t destination = (hdma_dest & 0x1FF0) | 0x8000;

	for (int i = 0; i < hdma_bytes; i++) {
		write_u8_ppu(destination + i, cpu->get_mmap().read_u8(source + i));
	}

	hdma_dest += hdma_bytes;
	hdma_source += hdma_bytes;

	for (int i = 0; i < 5; i++) {
		hdma[i] = 0xFF;
	}

	uint16_t clock_cycles = 0;

	if (cpu->get_cgb_speed())
		clock_cycles = 2 + 16 * ((value & 0x7f) + 1);
	else
		clock_cycles = 1 + 8 * ((value & 0x7f) + 1);
	cpu->set_cycle_16(clock_cycles * 4);
}