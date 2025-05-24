#include "ppm.h"
#include <fstream>
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