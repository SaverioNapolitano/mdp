#include "pgm16.h"

int main() {
	std::string filename = "CR-MONO1-10-chest.pgm";
	mat<uint16_t> img{};
	uint16_t maxvalue;
	bool res = load(filename, img, maxvalue);
	return EXIT_SUCCESS;
}