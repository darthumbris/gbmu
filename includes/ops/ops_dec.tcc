template <Registers rec>
void dec_r16() {
	set_16bitregister(rec, get_16bitregister(rec) - 1);
	set_cycle(2);
}

template <Registers rec>
void dec_r8() {
	uint8_t val;
	uint16_t res;
	if (rec == Registers::HL) {
		uint16_t addr = get_16bitregister(Registers::HL);
		val = mmap.read_u8(addr);
		res = val - 1;
		mmap.write_u8(addr, static_cast<uint8_t>(res));
		set_cycle(3);
	} else {
		val = get_register(rec);
		res = val - 1;
		set_register(rec, static_cast<uint8_t>(res));
		set_cycle(1);
	}
	set_flag(FlagRegisters::z, static_cast<uint8_t>(res) == 0);
	set_flag(FlagRegisters::n, 1);
	set_flag(FlagRegisters::h, (val & 0xf) < (1 & 0xf));
}