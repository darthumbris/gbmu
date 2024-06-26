template <Registers rec, Registers src>
void ld_r8_r8() {
	uint8_t val;
	if (src == Registers::HL) {
		val = mmap.read_u8(get_16bitregister(Registers::HL));
	} else {
		val = get_register(src);
	}
	if (rec == Registers::HL) {
		mmap.write_u8(get_16bitregister(Registers::HL), val);
	} else {
		set_register(rec, val);
	}
	set_cycle(1);
	if (src == Registers::HL || rec == Registers::HL) {
		set_cycle(1);
	}
}

template <Registers rec>
void ld_r16_a() {
	uint16_t address = get_16bitregister(rec);
	mmap.write_u8(address, get_register(Registers::A));
	set_cycle(2);
}

template <Registers src>
void ld_a_r16() {
	uint16_t address = get_16bitregister(src);
	set_register(Registers::A, mmap.read_u8(address));
	set_cycle(2);
}

template <Registers rec>
void ld_r8_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	set_register(rec, val);
	set_cycle(2);
}

void ld_hl_imm8() {
	uint8_t val = mmap.read_u8(pc);
	pc += 1;
	mmap.write_u8(get_16bitregister(Registers::HL), val);
	set_cycle(3);
}

template <Registers rec>
void ld_r16_imm16() {
	uint16_t val = mmap.read_u16(pc);
	pc += 2;
	set_16bitregister(rec, val);
	set_cycle(3);
}

void ld_a_imm16() {
	uint16_t addr = mmap.read_u16(pc);
	pc += 2;
	set_register(Registers::A, mmap.read_u8(addr));
	set_cycle(4);
}

void ld_imm16_a() {
	uint16_t addr = mmap.read_u16(pc);
	pc += 2;
	mmap.write_u8(addr, get_register(Registers::A));
	set_cycle(4);
}

void ld_imm16_sp() {
	uint16_t addr = mmap.read_u16(pc);
	pc += 2;
	mmap.write_u16(addr, sp);
	set_cycle(5);
}

void ld_hl_sp_imm8() {
	uint8_t e8 = mmap.read_u8(pc);
	pc += 1;
	uint16_t val = sp;
	set_16bitregister(Registers::HL, static_cast<uint16_t>(val + e8));
	set_flag(FlagRegisters::z, 0);
	set_flag(FlagRegisters::n, 0);
	set_flag(FlagRegisters::h, (val & 0xf) + (e8 & 0xf) > 0xf);
	set_flag(FlagRegisters::c, (val & 0xff) + (e8 & 0xff) > 0xff);
	set_cycle(3);
}

void ldh_a_imm8() {
	uint8_t addr = mmap.read_u8(pc);
	pc += 1;
	set_register(Registers::A, mmap.read_u8(0xFF00 + static_cast<uint16_t>(addr)));
	set_cycle(3);
}

void ldh_imm8_a() {
	uint8_t addr = mmap.read_u8(pc);
	pc += 1;
	mmap.write_u8(0xFF00 + static_cast<uint16_t>(addr), get_register(Registers::A));
	set_cycle(3);
}

void ld_c_a() {
	mmap.write_u8(0xFF00 + static_cast<uint16_t>(get_register(Registers::C)), get_register(Registers::A));
	set_cycle(2);
}

void ld_a_c() {
	set_register(Registers::A, mmap.read_u8(0xFF00 + static_cast<uint16_t>(get_register(Registers::C))));
	set_cycle(2);
}

void ld_sp_hl() {
	sp = get_16bitregister(Registers::HL);
	set_cycle(2);
}

void ld_i_hl_a() {
	mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl + 1);
	set_cycle(2);
}

void ld_i_a_hl() {
	set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl + 1);
	set_cycle(2);
}

void ld_d_hl_a() {
	mmap.write_u8(get_16bitregister(Registers::HL), get_register(Registers::A));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl - 1);
	set_cycle(2);
}

void ld_d_a_hl() {
	set_register(Registers::A, mmap.read_u8(get_16bitregister(Registers::HL)));
	uint16_t hl = get_16bitregister(Registers::HL);
	set_16bitregister(Registers::HL, hl - 1);
	set_cycle(2);
}

template <Registers rec>
INLINE_FN void pop_r16stk() {
	uint16_t val = sp;
	set_16bitregister(rec, mmap.read_u16(sp));
	sp += 2;
	set_cycle(3);
}

template <Registers src>
INLINE_FN void push_r16stk() {
	uint16_t val = get_16bitregister(src);
	sp -= 2;
	mmap.write_u16(sp, val);
	set_cycle(4);
}
