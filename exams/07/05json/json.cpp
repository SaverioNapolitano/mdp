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

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	int L = -1;
	std::vector<uint8_t> seen;
	int last_p = -1;
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			uint8_t p = img(r, c);
			if (p == last_p) { // run
				if (L == 0) { // thought it was a copy, instead is a run
					seen.clear();
					L = 256;
				}
				if (L >= 1 && L <= 127) { // was copying
					encoded.push_back(static_cast<uint8_t>(L - 1));
					encoded.insert(end(encoded), begin(seen), end(seen) - 1);
					seen.clear();
					L = 256;
				}
				if (L == 129) { // max run
					encoded.push_back(static_cast<uint8_t>(L));
					encoded.push_back(p);
					L = 256;
				}
				L--;
			}
			else { // copy
				if (L >= 129 && L <= 255) { // was running
					encoded.push_back(static_cast<uint8_t>(L));
					encoded.push_back(static_cast<uint8_t>(last_p));
					L = -1;
				}
				if (L == 127) { // max copy
					encoded.push_back(static_cast<uint8_t>(L));
					encoded.insert(end(encoded), begin(seen), end(seen));
					seen.clear();
					L = -1;
				}
				L++;
				seen.push_back(p);
			}
			last_p = p;
		}
	}
	if (L >= 0 && L <= 127) {
		encoded.push_back(static_cast<uint8_t>(L));
		encoded.insert(end(encoded), begin(seen), end(seen));
	}
	if (L >= 129 && L <= 255) {
		encoded.push_back(static_cast<uint8_t>(L));
		encoded.push_back(static_cast<uint8_t>(last_p));
	}
	encoded.push_back(128);
}

std::string Base64Encode(const std::vector<uint8_t>& v) {
	std::vector<char> table{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
	std::string encoded;
	for (size_t i = 0; i < v.size(); i += 3) {
		uint8_t first, second, third;
		first = v[i];
		second = i + 1 < v.size() ? v[i + 1] : 128;
		third = i + 2 < v.size() ? v[i + 2] : 128;
		uint8_t f = (first & 0xFC) >> 2;
		assert(f < 64);
		encoded.push_back(table[f]);
		uint8_t s = ((first & 0x03) << 4) | ((second & 0xF0) >> 4);
		assert(s < 64);
		encoded.push_back(table[s]);
		uint8_t t = ((second & 0x0F) << 2) | ((third & 0xC0) >> 6);
		assert(t < 64);
		encoded.push_back(table[t]);
		uint8_t l = third & 0x3F;
		assert(l < 64);
		encoded.push_back(table[l]);
	}
	return encoded;
}

std::string JSON(const std::string& filename) {
	mat<vec3b> img;
	if (!LoadPPM(filename, img)) {
		return "{}";
	}
	mat<uint8_t> img_r, img_g, img_b;
	SplitRGB(img, img_r, img_g, img_b);
	std::vector<uint8_t> encoded_r, encoded_g, encoded_b;
	PackBitsEncode(img_r, encoded_r);
	PackBitsEncode(img_g, encoded_g);
	PackBitsEncode(img_b, encoded_b);
	std::string base64_r, base64_g, base64_b;
	base64_r = Base64Encode(encoded_r);
	base64_g = Base64Encode(encoded_g);
	base64_b = Base64Encode(encoded_b);
	std::string str{ "{\n\t\"width\" : " + std::to_string(img.cols()) + ",\n\t\"height\" : " + std::to_string(img.rows()) + ",\n\t\"red\" : \"" +
		base64_r + "\",\n\t\"green\" : \"" + base64_g + "\",\n\t\"blue\" : " + base64_b + "\n}" };
	return str;
}