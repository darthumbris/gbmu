#ifndef OPERAND_HPP
#define OPERAND_HPP

#include <string>
#include "json.hpp"
#include <optional>

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
    BC = 8,
    DE = 10, 
    HL = 12, 
    AF = 14,
    SP = 16,
};

namespace Dict
{
    using json = nlohmann::json;
    class Operand
    {
    private:

    public:
        Operand();
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
