#include "pcx.h"

int main() {
	std::string s{ "gatto_colori_24bit.pcx" };
	mat<vec3b> img{};
	bool res = load_pcx(s, img);
	return EXIT_SUCCESS;
}