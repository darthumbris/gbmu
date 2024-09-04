#include "Cpu.hpp"
#include "debug.hpp"

#define OPTSTR "i:s:dc"
#define USAGE_FMT "%s [-i inputfile] [-d force-dmg-mode] [-c force cgb mode] [-s scale]\n"

int main(int argc, char *argv[]) {

	if (argc < 2) {
		ERROR_MSG(USAGE_FMT, argv[0]);
		exit(EXIT_FAILURE);
	}

	int opt;
	options options = {0, 0, "", 4};

	while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
		switch (opt) {
		case 'i':
			if (!std::ifstream(optarg, std::ios::binary).is_open()) {
				ERROR_MSG("Failed to open file: %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			options.path = optarg;
			break;
		case 'd':
			options.force_dmg = true;
			break;
		case 'c':
			options.force_cgb = true;
			break;
		case 's':
			options.scale = static_cast<uint8_t>(strtoul(optarg, nullptr, 10));
			if (options.scale > 8 || options.scale < 1) {
				ERROR_MSG("darkening options needs to be between 1-8\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			ERROR_MSG(USAGE_FMT, argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (options.path.empty()) {
		exit(EXIT_FAILURE);
	}

	Cpu cpu = Cpu(Dict::Decoder("Opcodes.json"), options);

	while (!cpu.status()) {
		cpu.tick();
	}
	cpu.close();
	return 0;
}
