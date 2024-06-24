template <Registers rec>
void dec_r16() {
	set_16bitregister(rec, get_16bitregister(rec) - 1);
	set_cycle(2);
}

template <Registers rec>
void dec_r8() {
	uint8_t val;
	if (rec == Registers::HL) {
		val = mmap.read_u8(get_16bitregister(Registers::HL));
		mmap.write_u8(get_16bitregister(Registers::HL), val - 1);
		set_flag(FlagRegisters::z, val == 0x01);
		set_flag(FlagRegisters::h, ((val & 0b00011111) == 0b00010000));
		set_cycle(3);
	} else {
		val = get_register(rec);
		set_register(rec, val - 1);
		set_flag(FlagRegisters::z, (val - 1) == 0x00);
		set_flag(FlagRegisters::h, (val & 0xF) == 0);
		set_cycle(1);
	}
	set_flag(FlagRegisters::n, 1);
}