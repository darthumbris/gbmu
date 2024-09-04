#include "Decoder.hpp"
#include <fstream>
#include <iostream>

namespace Dict {

	Decoder::Decoder(const std::string &path) {
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cerr << "Error: Failed to  open json file for instruction list." << std::endl;
			exit(EXIT_FAILURE);
		}
		try {
			json j = json::parse(f);
			json ins = j.at("unprefixed");
			for (auto it = ins.begin(); it != ins.end(); it++) {
				uint16_t x;
				std::stringstream ss;
				ss << std::hex << it.key();
				ss >> x;
				instructions.emplace(std::make_pair(x, Instruction(*it)));
			}
			json pref = j.at("cbprefixed");
			for (auto it = pref.begin(); it != pref.end(); it++) {
				uint16_t x;
				std::stringstream ss;
				ss << std::hex << it.key();
				ss >> x;
				prefixed_instructions.emplace(std::make_pair(x, Instruction(*it)));
			}
		} catch (const json::parse_error &e) {
			std::cout << e.what() << std::endl;
		}
		f.close();
	}

	Decoder::~Decoder() {}

}
