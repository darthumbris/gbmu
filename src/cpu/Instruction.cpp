#include "Instruction.hpp"
#include <iostream>

namespace Dict {

	Instruction::Instruction(const json &j) {
		mnemonic = j.at("mnemonic").get<std::string>();
		bytes = j.at("bytes").get<uint32_t>();
		cycles = j.at("cycles").get<std::vector<uint32_t>>();
		j.at("operands").get_to(operands);
		immediate = j.at("immediate").get<bool>();
		flags = j.at("flags").get<Flags>();
	}

	Instruction::Instruction() {
		mnemonic = "";
		bytes = 0;
		cycles = std::vector<uint32_t>();
		operands = std::vector<Operand>();
		immediate = false;
		flags = Flags();
	}

	Instruction::~Instruction() {}

	void Instruction::print_instruction() {
		std::string s;
		s = mnemonic;
		if (mnemonic.size() < 3)
			s.append(" ");
		s.append("\t");
		size_t i = 0;
		for (auto op : operands) {
			s.append(op.print());
			if (i < operands.size() - 1) {
				s.append(", ");
			}
			i += 1;
		}
		std::cout << s << std::endl;
	}

	void to_json(json &j, const Instruction &i) {
		j = json::object();
		j["bytes"] = i.bytes;
		j["cycles"] = i.cycles;
		j["operands"] = i.operands;
		j["immediate"] = i.immediate;
		j["flags"] = i.flags;
	}

	void from_json(const json &j, Instruction &i) {
		i.mnemonic = j.at("mnemonic").get<std::string>();
		i.bytes = j.at("bytes").get<uint32_t>();
		i.cycles = j.at("cycles").get<std::vector<uint32_t>>();
		j.at("operands").get_to(i.operands);
		i.immediate = j.at("immediate").get<bool>();
		i.flags = j.at("flags").get<Flags>();
	}

}