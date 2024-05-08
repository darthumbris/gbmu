#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <cstdint>
#include <vector>
#include "Flags.hpp"

namespace Dict
{

    struct Instruction
    {
        std::string mnemonic;
        uint8_t bytes;
        std::vector<uint8_t> cycles;
        std::vector<Operand> operands;
        bool immediate;
        Flags flags;
    };
    void to_json(json &j, const Instruction &x);
    void from_json(const json &j, Instruction &x);
}

#endif