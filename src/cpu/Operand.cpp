#include "Operand.hpp"
#include <iostream>

namespace Dict
{

    Operand::Operand(/* args */)
    {
        this->value = {};
    }

    Operand::Operand(const json &j)
    {
        Operand op;
        from_json(j, op);
        *this = op;
    }

    Operand::~Operand()
    {
    }

    std::string Operand::print() const
    {
        std::string v;
        std::string adjust;
        if (!decrement && !increment)
            adjust = "";
        else if (decrement)
            adjust = "-";
        else
            adjust = "+";
        if (value.has_value())
        {
            if (bytes > 0)
            {
                std::ostringstream vs;
                vs << "0x" << std::hex << std::uppercase << value.value();
                v = vs.str();
            }
            else
                v = value.value();
        }
        else
            v = name;
        v = v + adjust;
        if (immediate)
            return v;
        v.insert(0, "(");
        v.append(")");
        return v;
    }

    void to_json(json &j, const Operand &o)
    {
        j = json::object();
        j["name"] = o.name;
        j["immediate"] = o.immediate;
        j["bytes"] = o.bytes;
        j["increment"] = o.increment;
        j["decrement"] = o.decrement;
    }

    void from_json(const json &j, Operand &o)
    {
        o.name = j.at("name").get<std::string>();
        o.immediate = j.at("immediate").get<bool>();
        o.decrement = false;
        o.increment = false;
        o.bytes = 0;
        try
        {
            o.decrement = j.at("decrement").get<bool>();
        }
        catch (const std::exception &e)
        {
            // std::cout << e.what() << '\n';
        }
        try
        {
            o.increment = j.at("increment").get<bool>();
        }
        catch (const std::exception &e)
        {
            // std::cout << e.what() << '\n';
        }
        try
        {
            o.bytes = j.at("bytes").get<uint32_t>();
        }
        catch (const std::exception &e)
        {
            // std::cout << e.what() << '\n';
        }
        // o.decrement = j.at("decrement").get<bool>();
        // o.increment = j.at("increment").get<bool>();
        // o.bytes = j.at("increment").get<uint8_t>();
    }
}