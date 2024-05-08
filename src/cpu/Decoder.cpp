#include "Decoder.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <utility>
#include <sstream>

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
                instructions.emplace(std::make_pair(x, *it));
            }
            json pref = j.at("cbprefixed");
            for (auto it = pref.begin(); it != pref.end(); it++)
            {
                uint32_t x;
                std::stringstream ss;
                ss << std::hex << it.key();
                ss >> x;
                prefixed_instructions.emplace(std::make_pair(x, *it));
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

    int Decoder::read(int address, int count)
    {
        if (0 <= address + count <= data.size())
        {
            std::vector<std::byte> v(&data[address], &data[address + count]);
            std::cout << count << std::endl;
            return 1;
        }
        else
        {
            std::cout << "what the fuck" << std::endl;
            return 0;
        }
    }

    std::pair<uint32_t, Instruction> Decoder::decode(uint32_t address)
    {
        uint32_t opcode;
        Instruction instruction;
        opcode = read(address);
        address += 1;

        if (opcode == 0xCB)
        {
            opcode = read(address);
            address += 1;
            instruction = prefixed_instructions[opcode];
        }
        else
        {
            instruction = instructions[opcode];
        }

        std::vector<Operand> ops;
        for (auto op : instruction.operands)
        {
            if (op.bytes)
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
        return std::make_pair(address, decoded);
    }

    void Decoder::disassemble(int address, int count)
    {
        for (int i = 0; i < count; i++)
        {
            std::pair<uint32_t, Instruction> dec = decode(address);
            // dec .1.print();
            std::string s;
            for (auto op : dec.second.operands)
            {
                s.append(op.name).append(",");
            }
            s.append(dec.second.mnemonic);
            std::cout << s << std::endl;
            address = dec.first;
        }
    }

}
