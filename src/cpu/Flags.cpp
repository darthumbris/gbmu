#include "Flags.hpp"

namespace Dict
{

    Flags::Flags()
    {
    }

    Flags::~Flags()
    {
    }

    void to_json(json &j, const Flags &f)
    {
        j = json::object();
        j["Z"] = f.z;
        j["N"] = f.n;
        j["H"] = f.h;
        j["C"] = f.c;
    }

    void from_json(const json &j, Flags &f)
    {
        f.z = j.at("Z").get<std::string>();
        f.n = j.at("N").get<std::string>();
        f.h = j.at("H").get<std::string>();
        f.c = j.at("C").get<std::string>();
    }
}