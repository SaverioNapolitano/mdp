#include <cstdint>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <print>
#include <ostream>
#include <cassert>
#include <map>

using rgb = std::array<uint8_t, 3>;
struct image {
	size_t width_ = 0;
	size_t height_ = 0;
	std::vector<rgb> data_;

	image() {}
	image(size_t width, size_t height) : width_(width), height_(height), data_(width* height){}

	rgb& operator()(size_t r, size_t c) {
		return data_[r * width_ + c];
	}

	rgb& operator[](size_t i) {
		return data_[i];
	}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rawsize() {
		return width_ * height_ * sizeof(rgb);
	}

};

image read_image(std::ifstream& is) {
	std::string magic_number(2, ' ');
	is >> magic_number;
	if (magic_number != "P6") {
		return image();
	}
	is.get(); // whitespace
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	size_t width = 0, height = 0;
	is >> width;
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	is >> height;
	if (width == 0 || height == 0) {
		return image();
	}
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	uint16_t maxval = 0;
	is >> maxval;
	if (maxval == 0) {
		return image();
	}
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	is.get(); // whitespace character
	image im(width, height);
	is.read(im.rawdata(), im.rawsize());
	
	return im;
}
void write_image(std::ostream& os, image& im) {
	std::print(os, "P6\n{} {}\n255\n", im.width_, im.height_);
	os.write(im.rawdata(), im.rawsize());
}

char find_char(uint32_t r, size_t n, size_t times) {
	std::array<char, 85> table{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/',
		'*', '?', '&', '<', '>', '(', ')', '[', ']', '{',
		'}', '@', '%', '$', '#' };
	int32_t p = r - n * times;
	if (p < 0) {
		int d = ((p * (-1)) / 85);
		p += d * 85;
		if (p < 0) {
			p += 85;
		}
	}
	assert(p < 85 && p >= 0);
	return table[p];

}
std::string convert(uint32_t octets, size_t n, size_t& times) {
	std::string token(5, '0');
	size_t i = 4;
	std::array<uint32_t, 5> remainders{ 0, 0, 0, 0, 0 };
	while (octets > 0) {
		uint32_t r = octets % 85;
		octets /= 85;
		remainders[i--] = r;
	}
	for (size_t j = 0; j < 5; j++) {
		token[j] = find_char(remainders[j], n, times);
		times++;
	}
	return token;
}
void encode(image& im, std::ostream& os, size_t n) {
		uint32_t octets = 0;
		uint8_t n_octets = 0;
		size_t j = 0;
		size_t times = 0;
		for (size_t i = 0; i < im.height_ * im.width_; i++) {
			rgb pixel = im[i];
			while (n_octets < 4 && j < 3) {
				octets = (octets << 8) | pixel[j++];
				n_octets++;
			}
			if (n_octets < 4) {
				if (i < (im.width_ * im.height_ - 1)) {
					pixel = im[i + 1];
					j = 0;
					while (n_octets < 4 && j < 3) {
						octets = (octets << 8) | pixel[j++];
						n_octets++;
					}
					if (n_octets == 4) {
						std::string token = convert(octets, n, times);
						os.write(token.data(), token.size());
						octets = 0;
						n_octets = 0;
					}
				}
				else {
					if (n_octets > 0) {
						while (n_octets < 4) {
							octets = (octets << 8);
							n_octets++;
						}
						std::string token = convert(octets, n, times);
						os.write(token.data(), token.size());
					}
				}
			}
			else {
				assert(n_octets == 4);
				std::string token = convert(octets, n, times);
				os.write(token.data(), token.size());
				octets = 0;
				n_octets = 0;
			}
		}
}
void compress(size_t n, std::ifstream& is, std::ofstream& os) {
	image im = read_image(is);
	std::print(os, "{},{},", im.width_, im.height_);
	encode(im, os, n);
}

uint32_t find_value(char c, size_t n, size_t times) {
	std::array<char, 85> table{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
		'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', '.', '-', ':', '+', '=', '^', '!', '/',
		'*', '?', '&', '<', '>', '(', ')', '[', ']', '{',
		'}', '@', '%', '$', '#' };

	for (int32_t i = 0; i < 85; i++) {
		if (table[i] == c) {	
			i += n * times;
			while (i >= 85) {
				i -= 85;
			}
			return i;
		}
	}
	return -1;
}
uint32_t parse(std::string token, size_t n, size_t& times) {
	uint32_t value = 0;
	uint32_t q = 0;
	for (size_t i = 0; i < token.size(); i++) {
		uint32_t r = find_value(token[i], n, times);
		times++;
		value = q * 85 + r;
		q = value;
	}
	return value;
}
void decompress(size_t n, std::ifstream& is, std::ofstream& os) {
	size_t width, height;
	is >> width;
	is.get(); // ','
	is >> height;
	is.get(); // '.'
	image im(width, height);
	std::string token(5, ' ');
	std::vector<uint8_t> bytes;
	is >> token;
	size_t pos = 0;
	size_t times = 0;
	while (pos < token.size()) {
		uint32_t value = parse(token.substr(pos, 5), n, times);
		bytes.push_back(value >> 24);
		bytes.push_back(value >> 16);
		bytes.push_back(value >> 8);
		bytes.push_back(value);
		pos += 5;
	}
	size_t index = 0;
	for (size_t i = 0; index < width * height; i += 3) {
		im[index++] = { bytes[i], bytes[i + 1], bytes[i + 2] };
	}
	write_image(os, im);
}
int main(int argc, char** argv) {
	using namespace std::string_literals;
	if (argc != 5) {
		return EXIT_FAILURE;
	}
	if (argv[1] == "c"s) {
		size_t n = std::stoul(argv[2]);
		std::ifstream is(argv[3], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		std::ofstream os(argv[4]/*, std::ios::binary*/);
		if (!os) {
			return EXIT_FAILURE;
		}
		compress(n, is, os);
	}
	else if (argv[1] == "d"s) {
		size_t n = std::stoul(argv[2]);
		std::ifstream is(argv[3]/*, std::ios::binary*/);
		if (!is) {
			return EXIT_FAILURE;
		}
		std::ofstream os(argv[4], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}
		decompress(n, is, os);
	}
	else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}