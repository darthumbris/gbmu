#include "Cpu.hpp"
#include "Decoder.hpp"
#include <SDL2/SDL.h>

#define OPTSTR "i:d:fmcs"
#define USAGE_FMT "%s [-i inputfile] [-f force-dmg-mode] [-s force cgb mode] [-m matrix/raster] [-d darkening %%] [-c color-correction]\n"
#define DEFAULT_PROGNAME "gbmu"

void usage(char *progname) {
	fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		usage(basename(argv[0]));
	}

	int opt;
	options options = {0, 0, 0, 0, 0, ""};

	while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
		switch (opt) {
		case 'i':
			if (!std::ifstream(optarg, std::ios::binary).is_open()) {
				exit(EXIT_FAILURE);
			}
			options.path = optarg;
			break;
		case 'f':
			options.force_dmg = true;
			break;
		case 's':
			options.force_cgb = true;
			break;
		case 'm':
			options.matrix = true;
			break;
		case 'c':
			options.color_correction = true;
			break;
		case 'd':
			options.darkening = static_cast<uint8_t>(strtoul(optarg, NULL, 10));
			if (options.darkening > 100) {
				printf("darkening options needs to be between 0-100\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			usage(basename(argv[0]));
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
