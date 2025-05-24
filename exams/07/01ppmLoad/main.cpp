#include "ppm.h"

extern bool LoadPPM(const std::string& filename, mat<vec3b>& img);

int main() {
	std::string filename("facolta.ppm");
	mat<vec3b> img;
	bool res = LoadPPM(filename, img);
	return EXIT_SUCCESS;
}
