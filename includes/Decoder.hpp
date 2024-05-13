#ifndef DECODER_HPP
#define DECODER_HPP

#include <string>
#include <vector>
#include <map>
#include "Instruction.hpp"
#include <cstddef>

namespace Dict
{
    class Decoder
    {
    private:
        std::map<uint32_t, Instruction> prefixed_instructions;
        std::map<uint32_t, Instruction> instructions;

        std::vector<std::byte> data;

    public:
        Decoder(std::string &path);
        ~Decoder();

        void set_data(std::string path);

        int read(int address, int count = 1);
        std::pair<uint32_t, Instruction> decode(uint32_t address);
        void disassemble(int address, int count);
    };

}

#endif
