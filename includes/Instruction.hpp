#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include "Flags.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace Dict {

	class Instruction {
	private:
		/* data */
	public:
		Instruction(const json &j);
		Instruction();
		~Instruction();

		void print_instruction();

		std::string mnemonic;
		uint32_t bytes;
		std::vector<uint32_t> cycles;
		std::vector<Operand> operands;
		bool immediate;
		Flags flags;
	};

	void to_json(json &j, const Instruction &i);
	void from_json(const json &j, Instruction &i);

}

#endif