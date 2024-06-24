#ifndef FLAGS_HPP
#define FLAGS_HPP

#include "Operand.hpp"
#include <string>

namespace Dict {

	class Flags {
	private:
	public:
		Flags();
		~Flags();

		std::string z;
		std::string n;
		std::string h;
		std::string c;
	};
	void from_json(const json &j, Flags &x);
	void to_json(json &j, const Flags &x);

}

#endif