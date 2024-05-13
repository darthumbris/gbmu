#ifndef OPERAND_HPP
#define OPERAND_HPP

#include <string>
#include "json.hpp"
#include <optional>

using json = nlohmann::json;

namespace Dict
{
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
        uint32_t bytes;
        bool decrement;
        bool increment;
        std::optional<uint32_t> value;
    };

    void from_json(const json &j, Operand &o);

    void to_json(json &j, const Operand &o);
}

#endif
