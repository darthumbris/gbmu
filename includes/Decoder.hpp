#ifndef DECODER_HPP
#define DECODER_HPP

#include "Instruction.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace Dict {

	class Decoder {
	private:
		std::vector<std::byte> data;

	public:
		Decoder(const std::string &path);
		~Decoder();

		std::map<uint16_t, Instruction> prefixed_instructions;
		std::map<uint16_t, Instruction> instructions;
	};

}

#endif
