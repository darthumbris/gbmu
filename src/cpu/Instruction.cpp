#include "Instruction.hpp"
#include <iostream>

namespace Dict
{
    void to_json(json &j, const Instruction &i)
    {
        j = json::object();
        j["bytes"] = i.bytes;
        j["cycles"] = i.cycles;
        j["operands"] = i.operands;
        j["immediate"] = i.immediate;
        j["flags"] = i.flags;
    }

    void from_json(const json &j, Instruction &i)
    {
        i.mnemonic = j.at("mnemonic").get<std::string>();
        i.bytes = j.at("bytes").get<uint32_t>();
        i.cycles = j.at("cycles").get<std::vector<uint32_t>>();
        j.at("operands").get_to(i.operands);
        i.immediate = j.at("immediate").get<bool>();
        i.flags = j.at("flags").get<Flags>();
    }
}