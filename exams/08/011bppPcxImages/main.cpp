#include "pcx.h"

int main() {
	std::string s{ "bunny.pcx" };
	mat<uint8_t> img{};
	bool res = load_pcx(s, img);
	return EXIT_SUCCESS;
}