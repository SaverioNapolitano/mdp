#include "ppm.h"

extern void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b);
extern bool LoadPPM(const std::string& filename, mat<vec3b>& img);
extern void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded);

int main() {
	std::string filename("facolta.ppm");
	mat<vec3b> img;
	mat<uint8_t> img_r, img_g, img_b;
	bool res = LoadPPM(filename, img);
	SplitRGB(img, img_r, img_g, img_b);
	std::vector<uint8_t> encoded;
	PackBitsEncode(img_r, encoded);
	return EXIT_SUCCESS;
}