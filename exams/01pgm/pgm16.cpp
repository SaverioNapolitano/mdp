#include "pgm16.h"
#include <fstream>


void read_comment(std::istream& is) {
	char c;
	while (is.get(c)) {
		if (c == 0x0A) {
			break;
		}
	}
}

unsigned long read_metadata(std::istream& is, char delimiter) {
	std::string s{};
	char c;
	while (is.get(c)) {
		if (c == delimiter) {
			break;
		}
		s.append(1, c);
	}
	size_t pos;
	return std::stoul(s, &pos);
}

void read_row(std::istream& is, mat<uint16_t>& img, int nbytes, int r, int cols) {
	uint8_t b1 = 0;
	uint8_t b2 = 0;
	uint16_t p = 0;
	char ch;
	for (int c = 0; c < cols; c++) {
		if (nbytes == 1) {
			is.get(ch);
			b1 = ch;
			p = b1;
		}
		else {
			is.get(ch);
			b1 = ch;
			is.get(ch);
			b2 = ch;
			p = b1;
			p = (p << 8) | b2;
		}
		img.data()[r * cols + c] = p;
	}
}

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	char c;
	is.get(c);
	if (c != 'P') {
		return false;
	}
	is.get(c);
	if (c != '5') {
		return false;
	}
	is.get(c); // \n
	is.get(c);
	if (c == '#') {
		read_comment(is);
	}
	unsigned long W = read_metadata(is, ' ');
	unsigned long H = read_metadata(is, 0x0A);
	maxvalue = read_metadata(is, 0x0A);
	int nbytes = maxvalue < 256 ? 1 : 2;
	img.resize(static_cast<int>(H), static_cast<int>(W));
	for (int r = 0; r < img.rows(); r++) {
		read_row(is, img, nbytes, r, img.cols());
	}

	return true;

}