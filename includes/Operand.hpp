#ifndef OPERAND_HPP
#define OPERAND_HPP

#include <string>
#include "json.hpp"
#include <optional>

// enum LowRegisters
// {
//     L_C = 1,
//     L_E = 3,
//     L_L = 5,
//     L_F = 7,
// };

// enum HighRegisters
// {
//     H_B = 0,
//     H_D = 2,
//     H_H = 4,
//     H_A = 6,
// };

enum Registers
{
    B = 0,
    C = 1,
    D = 2,
    E = 3,
    H = 4,
    L = 5,
    A = 6,
    F = 7,
    BC = 8, //8-8 > 0-1
    DE = 10, //10-8 -> 2-3
    HL = 12, //12-8 ->4-5
    AF = 14,//14-8 ->6-7
    SP = 16, //sp
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
