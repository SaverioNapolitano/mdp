#include "pbm.h"

void BinaryImage::read_comment(std::istream& is) {
	char c;
	while (is.get(c)) {
		if (c == 0x0A) {
			break;
		}
	}
}

void BinaryImage::read_width(std::istream& is) {
	std::string s{};
	char c;
	while (is.get(c)) {
		if (c == '#') {
			read_comment(is);
		}
		if (c == ' ') {
			break;
		}
		s.append(1, c);
	}
	size_t pos;
	BinaryImage::W = static_cast<int>(std::stoul(s, &pos));
}
void BinaryImage::read_height(std::istream& is) {
	std::string s{};
	char c;
	while (is.get(c)) {
		if (c == 0x0A) {
			break;
		}
		s.append(1, c);
	}
	size_t pos;
	BinaryImage::H = static_cast<int>(std::stoul(s, &pos));
}

void BinaryImage::read_row(std::istream& is, int cols) {
	char ch;
	uint8_t u;
	for (int c = 0; c < cols; c+=8) {
		is.get(ch);
		u = ch;
		ImageData.push_back(u);
	}
	
}

bool BinaryImage::ReadFromPBM(const std::string& filename) {
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
	if (c != '4') {
		return false;
	}
	is.get(c); // \n
	read_width(is);
	read_height(is);
	for (int r = 0; r < BinaryImage::H; r++) {
		read_row(is, BinaryImage::W);
	}
	return true;

}