template <Condition condition>
void call_cond_imm16() {
	pc += 2;
	bool cond = false;
	switch (condition) {
	case Condition::NotZeroFlag:
		if (get_flag(FlagRegisters::z)) {
			cond = true;
		}
		break;
	case Condition::ZeroFlag:
		if (!get_flag(FlagRegisters::z)) {
			cond = true;
		}
		break;
	case Condition::NotCarryFlag:
		if (get_flag(FlagRegisters::c)) {
			cond = true;
		}
		break;
	case Condition::CarryFlag:
		if (!get_flag(FlagRegisters::c)) {
			cond = true;
		}
		break;
	default:
		break;
	}
	if (cond) {
		set_cycle(3);
		return;
	}
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = mmap.read_u16(pc - 2);
	set_cycle(6);
}

void call_imm16() {
	pc += 2;
	sp -= 2;
	mmap.write_u16(sp, pc);
	pc = mmap.read_u16(pc - 2);
	set_cycle(6);
}