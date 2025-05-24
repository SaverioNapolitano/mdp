#include "ppm.h"
#include <fstream>
#include <algorithm>
void check_comment(std::istream& is) {
	is.get();
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
}
bool LoadPPM(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}
	std::string token(2, ' ');
	is >> token;
	if (token != "P6") {
		return false;
	}
	check_comment(is);
	int width = 0, height = 0, maxval = 0;
	is >> width;
	if (width == 0) {
		return false;
	}
	check_comment(is);
	is >> height;
	if (height == 0) {
		return false;
	}
	check_comment(is);
	is >> maxval;
	if (maxval == 0 || maxval >= 65536) {
		return false;
	}
	is.get();
	img.resize(height, width);
	is.read(img.rawdata(), img.rawsize());
	return true;
}

void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
	img_r.resize(img.rows(), img.cols());
	img_g.resize(img.rows(), img.cols());
	img_b.resize(img.rows(), img.cols());
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			vec3b pixel = img(r, c);
			img_r(r, c) = pixel[0];
			img_g(r, c) = pixel[1];
			img_b(r, c) = pixel[2];
		}
	}
}