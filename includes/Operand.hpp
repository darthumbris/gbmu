#ifndef OPERAND_HPP
#define OPERAND_HPP

#include <string>
#include "json.hpp"
#include <optional>

enum LowRegisters
{
    L_C = 1,
    L_E = 3,
    L_L = 5,
    L_F = 7,
};

enum HighRegisters
{
    H_B = 0,
    H_D = 2,
    H_H = 4,
    H_A = 6,
};

enum Registers
{
    B = HighRegisters::H_B,
    C = LowRegisters::L_C,
    D = HighRegisters::H_D,
    E = LowRegisters::L_E,
    H = HighRegisters::H_H,
    L = LowRegisters::L_L,
    A = HighRegisters::H_A,
    F = LowRegisters::L_F,
    BC = 8,
    DE = 9,
    HL = 10,
    AF = 11,
    SP = 12,
};

namespace Dict
{
    using json = nlohmann::json;
    class Operand
    {
    private:
        /* data */
    public:
        Operand(/* args */);
        Operand(const json &j);
        ~Operand();

        std::string print() const;

        bool immediate;
        std::string name;
        Registers reg;
        uint32_t bytes;
        bool decrement;
        bool increment;
        std::optional<uint32_t> value;
    };

    void from_json(const json &j, Operand &o);

    void to_json(json &j, const Operand &o);
}

#endif
