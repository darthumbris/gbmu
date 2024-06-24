uint8_t get_rlc(uint8_t val, bool reset = false) {
	uint8_t ret = (val << 1) | (val >> 7);
	set_flag(FlagRegisters::c, (ret >> 0) & 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

uint8_t get_rrc(uint8_t val, bool reset = false) {
	uint8_t ret = (val >> 1) | (val << 7);
	set_flag(FlagRegisters::c, (ret >> 7) & 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

uint8_t get_rr(uint8_t val, bool reset = false) {
	uint8_t ret = (val >> 1) | (get_flag(FlagRegisters::c) << 7);
	set_flag(FlagRegisters::c, val & 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

uint8_t get_rl(uint8_t val, bool reset = false) {
	uint8_t ret;
	ret = (val << 1) | (get_flag(FlagRegisters::c) << 0);
	set_flag(FlagRegisters::c, ((val & 0x80) >> 7) == 1);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	if (reset) {
		set_flag(FlagRegisters::z, 0);
	} else {
		set_flag(FlagRegisters::z, ret == 0);
	}
	return ret;
}

void rlca() {
	set_register(Registers::A, get_rlc(get_register(Registers::A), true));
	set_cycle(1);
}

void rla() {
	set_register(Registers::A, get_rl(get_register(Registers::A), true));
	set_cycle(1);
}

void rrca() {
	set_register(Registers::A, get_rrc(get_register(Registers::A), true));
	set_cycle(1);
}

void rra() {
	set_register(Registers::A, get_rr(get_register(Registers::A), true));
	set_cycle(1);
}

template <Registers rec>
void rlc_r8() {
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), get_rlc(mmap.read_u8(get_16bitregister(Registers::HL))));
		set_cycle(4);
	} else {
		set_register(rec, get_rlc(get_register(rec)));
		set_cycle(2);
	}
}

template <Registers rec>
void rl_r8() {
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), get_rl(mmap.read_u8(get_16bitregister(Registers::HL))));
		set_cycle(4);
	} else {
		set_register(rec, get_rl(get_register(rec)));
		set_cycle(2);
	}
}

template <Registers rec>
void rrc_r8() {
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), get_rrc(mmap.read_u8(get_16bitregister(Registers::HL))));
		set_cycle(4);
	} else {
		set_register(rec, get_rrc(get_register(rec)));
		set_cycle(2);
	}
}

template <Registers rec>
void rr_r8() {
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), get_rr(mmap.read_u8(get_16bitregister(Registers::HL))));
		set_cycle(4);
	} else {
		set_register(rec, get_rr(get_register(rec)));
		set_cycle(2);
	}
}

template <Registers rec>
void sla_r8() {
	uint8_t val;
	if (rec == Registers::HL) {
		val = mmap.read_u8(get_16bitregister(Registers::HL));
		set_cycle(4);
	} else {
		val = get_register(rec);
		set_cycle(2);
	}
	set_flag(FlagRegisters::c, (val >> 7) & 0x01);
	val <<= 1;
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::z, val == 0);
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), val);
	} else {
		set_register(rec, val);
	}
}

template <Registers rec>
void sra_r8() {
	uint8_t val;
	if (rec == Registers::HL) {
		val = mmap.read_u8(get_16bitregister(Registers::HL));
		set_cycle(4);
	} else {
		val = get_register(rec);
		set_cycle(2);
	}
	set_flag(FlagRegisters::c, (val >> 0) & 1);
	val = (val >> 1) | (val & 0x80);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::z, val == 0);
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), val);
	} else {
		set_register(rec, val);
	}
}

template <Registers rec>
void srl_r8() {
	uint8_t val;
	if (rec == Registers::HL) {
		val = mmap.read_u8(get_16bitregister(Registers::HL));
		set_cycle(4);
	} else {
		val = get_register(rec);
		set_cycle(2);
	}
	set_flag(FlagRegisters::c, val & 0x01);
	val >>= 1;
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, 0);
	set_flag(FlagRegisters::z, val == 0);
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), val);
	} else {
		set_register(rec, val);
	}
}
