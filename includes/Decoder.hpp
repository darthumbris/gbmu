#ifndef DECODER_HPP
#define DECODER_HPP

#include <string>
#include <vector>
#include <map>
#include "Instruction.hpp"
#include <cstddef>

template <typename IntegerType>
void bitsToInt(IntegerType &result, const char *bits, bool little_endian = true);

namespace Dict
{
    class Decoder
    {
    private:
        std::vector<std::byte> data;

    public:
        Decoder(std::string &path);
        ~Decoder();

        void set_data(std::string path);
        std::vector<std::byte> get_data() const;

        int read(int address, int count = 1);
        std::tuple<uint32_t, Instruction, bool> decode(uint32_t address);
        void disassemble(int address, int count);

        std::map<uint32_t, Instruction> prefixed_instructions;
        std::map<uint32_t, Instruction> instructions;
    };

}

#endif
