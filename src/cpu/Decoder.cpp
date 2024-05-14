#include "Decoder.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <utility>
#include <sstream>

template <typename IntegerType>
void bitsToInt(IntegerType &result, const char *bits, bool little_endian)
{
    result = 0;
    if (little_endian)
        for (int n = sizeof(result) - 1; n >= 0; n--)
            result = (result << 8) + (uint8_t)bits[n];
    else
        for (int n = 0; n < sizeof(result); n++)
            result = (result << 8) + (uint8_t)bits[n];
}

namespace Dict
{

    Decoder::Decoder(std::string &path)
    {
        std::cout << "\n==================opened stream==================" << std::endl;
        std::ifstream f(path);
        try
        {
            json j = json::parse(f);
            json ins = j.at("unprefixed");
            for (auto it = ins.begin(); it != ins.end(); it++)
            {
                uint32_t x;
                std::stringstream ss;
                ss << std::hex << it.key();
                ss >> x;
                instructions.emplace(std::make_pair(x, Instruction(*it)));
            }
            json pref = j.at("cbprefixed");
            for (auto it = pref.begin(); it != pref.end(); it++)
            {
                uint32_t x;
                std::stringstream ss;
                ss << std::hex << it.key();
                ss >> x;
                instructions.emplace(std::make_pair(x, Instruction(*it)));
            }
        }
        catch (const json::parse_error &e)
        {
            std::cout << e.what() << std::endl;
        }
        std::cout << "instructions: " << instructions.size() << std::endl;
        std::cout << "prefixed_instructions: " << prefixed_instructions.size() << std::endl;
    }

    Decoder::~Decoder()
    {
    }

    void Decoder::set_data(std::string path)
    {
        std::ifstream ifs;
        ifs.open(path.c_str(), std::ifstream::binary);
        auto size = ifs.tellg();
        while (!ifs.eof())
        {
            char c = ifs.get();
            data.push_back(static_cast<std::byte>(c));
        }
        ifs.close();
    }

    int Decoder::read(int address, int count)
    {
        if (0 <= address + count <= data.size())
        {
            std::vector<std::byte> v(&data[address], &data[address + count]);
            int ret;
            char dat[4];
            int i = 0;
            for (i = 0; i < count; i++)
            {
                dat[i] = static_cast<char>(v[i]);
            }
            for (; i < 4; i++)
            {
                dat[i] = 0;
            }
            bitsToInt(ret, dat, true);
            return ret;
        }
        else
        {
            std::cout << "what the fuck" << std::endl;
            return 0;
        }
    }

    std::tuple<uint32_t, Instruction, bool> Decoder::decode(uint32_t address)
    {
        uint32_t opcode;
        Instruction instruction;
        opcode = read(address);
        address += 1;
        bool prefix = false;
        if (opcode == 0xCB)
        {
            opcode = read(address);
            address += 1;
            instruction = prefixed_instructions[opcode];
            prefix = true;
        }
        else
        {
            instruction = instructions[opcode];
        }

        std::vector<Operand> ops;
        for (auto op : instruction.operands)
        {
            if (op.bytes > 0)
            {
                uint32_t value = read(address, op.bytes);
                address += op.bytes;
                Operand nop = op;
                nop.value = value;
                ops.push_back(nop);
            }
            else
            {
                ops.push_back(op);
            }
        }
        Instruction decoded = instruction;
        decoded.operands = ops;
        return {address, decoded, prefix};
    }

    void Decoder::disassemble(int address, int count)
    {
        for (int i = 0; i < count; i++)
        {
            auto dec = decode(address);
            std::cout << "0" << std::hex << address << " ";
            std::get<1>(dec).print_instruction();
            address = std::get<0>(dec);
        }
    }

}
