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
	for (int c = 0; c < cols; c += 8) {
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

Image BinaryImageToImage(const BinaryImage& bimg) {
	Image im{};
	im.H = bimg.H;
	im.W = bimg.W;
	int total_read_pixels = 0;
	int row_read_pixels = 0;
	bool enough = false;
	for (size_t i = 0; i < bimg.ImageData.size(); i++) {
		uint8_t byte = bimg.ImageData[i];
		for (size_t i = 0; i < 8; i++) {
			uint8_t bit = (byte >> i) & 1;
			uint8_t value = bit == 0 ? 0 : 255;
			im.ImageData.push_back(value);
			++total_read_pixels;
			++row_read_pixels;
			if (total_read_pixels == im.H * im.W) {
				enough = true;
				break;
			}
			if (row_read_pixels == bimg.W) {
				row_read_pixels = 0;
				break;
			}
		}
		if (enough) {
			break;
		}
	}
	return im;
}