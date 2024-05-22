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

    inline constexpr auto string_hash(const std::string_view sv)
    {
        unsigned long hash{5381};
        for (unsigned char c : sv)
        {
            hash = ((hash << 5) + hash) ^ c;
        }
        return hash;
    }

    inline constexpr auto operator"" _(const char *str, size_t len)
    {
        return string_hash(std::string_view{str, len});
    }

    Registers get_register(std::string name)
    {
        switch (string_hash(name))
        {
        case "B"_:
            return Registers::B;
        case "C"_:
            return Registers::C;
        case "D"_:
            return Registers::D;
        case "E"_:
            return Registers::E;
        case "H"_:
            return Registers::H;
        case "L"_:
            return Registers::L;
        case "A"_:
            return Registers::A;
        case "F"_:
            return Registers::F;
        case "BC"_:
            return Registers::BC;
        case "DE"_:
            return Registers::DE;
        case "HL"_:
            return Registers::HL;
        case "AF"_:
            return Registers::AF;
        case "SP"_:
            return Registers::SP;
        // case "n16"_:
        //     return Registers::SP; // TODO
        // case "n8"_:
        //     return Registers::SP; // TODO
        // case "e8"_:
        //     return Registers::SP; // TODO 8-bit signed 2's complement displacement.
        // case "a16"_:
        //     return Registers::SP; // TODO
        // case "a8"_:
        //     return Registers::SP; // TODO
        // case "NZ"_:
        //     return Registers::SP; // TODO not zero flag
        // case "Z"_:
        //     return Registers::SP; // TODO zero flag
        // case "NC"_:
        //     return Registers::SP; // TODO not carry flag
        // case "C"_:
        //     return Registers::SP; // TODO carry flag
        default:
            // std::cout << "name operand not implemented: " << name << std::endl;
            break;
        }
        return Registers::SP;
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
        o.reg = get_register(o.name);
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
        if (o.name.starts_with("$"))
        {
            o.value = std::stoi(o.name.substr(1, 2), 0, 16);
        }
    }
}
