#include "pcx.h"

int main() {
	std::string s{ "islanda_colori_8bit.pcx" };
	mat<vec3b> img{};
	bool res = load_pcx(s, img);
	return EXIT_SUCCESS;
}