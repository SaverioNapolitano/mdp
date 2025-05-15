#include <fstream>
#include <string>
#include <cstdint>
#include <cmath>
#include <array>
#include <vector>
#include <print>
#include <ostream>
#include <bit>
#include <cassert>

using bgr = std::array<uint8_t, 3>;
using entry = std::array<uint8_t, 4>;

bool read_file_header(std::istream& is) {
	std::string token(2, ' ');
	is.read(token.data(), 2);
	if (token != "BM") {
		return false;
	}
	uint32_t size;
	is.read(reinterpret_cast<char*>(&size), 4);
	uint16_t reserved1, reserved2;
	is.read(reinterpret_cast<char*>(&reserved1), 2);
	is.read(reinterpret_cast<char*>(&reserved2), 2);
	uint32_t image_data_address;
	is.read(reinterpret_cast<char*>(&image_data_address), 4);
	return true;
}

bool read_info_header(std::istream& is, uint16_t& bpp, int32_t& width, int32_t& height, uint32_t& num_colors) {
	uint32_t header_size;
	is.read(reinterpret_cast<char*>(&header_size), 4);
	if (header_size != 40) {
		return false;
	}
	is.read(reinterpret_cast<char*>(&width), 4);
	is.read(reinterpret_cast<char*>(&height), 4);
	uint16_t color_planes;
	is.read(reinterpret_cast<char*>(&color_planes), 2);
	if (color_planes != 1) {
		return false;
	}
	is.read(reinterpret_cast<char*>(&bpp), 2);
	uint32_t compression_method;
	is.read(reinterpret_cast<char*>(&compression_method), 4);
	if (compression_method != 0) {
		return false;
	}
	uint32_t image_size;
	is.read(reinterpret_cast<char*>(&image_size), 4);
	int32_t horz_resol, vert_resol;
	is.read(reinterpret_cast<char*>(&horz_resol), 4);
	is.read(reinterpret_cast<char*>(&vert_resol), 4);
	is.read(reinterpret_cast<char*>(&num_colors), 4);
	if (num_colors == 0) {
		num_colors = static_cast<uint32_t>(pow(2, bpp));
	}
	uint32_t imp_colors;
	is.read(reinterpret_cast<char*>(&imp_colors), 4);
	return true;
}

std::vector<entry> read_color_table(std::istream& is, uint32_t num_colors) {
	std::vector<entry> table;
	for (uint32_t i = 0; i < num_colors; i++) {
		uint8_t b = is.get();
		uint8_t g = is.get();
		uint8_t r = is.get();
		uint8_t o = is.get();
		table.push_back({ b, g, r, o });
	}
	return table;
}

std::vector<bgr> read_image(std::ifstream& is, uint16_t& bpp, int32_t width, int32_t height, std::vector<entry>& table) {
	std::vector<bgr> im(width * height);
	int32_t read_pixels = 0;
	uint8_t b, g, r;
	uint32_t dummy;
	uint32_t pad = 0;
	if ((width * 8) % 32 != 0) {
		uint32_t d = (width * 8) / 32;
		pad = ((d + 1) * 32 - (width * 8)) / 8;
	}
	size_t index = 0;
	while (read_pixels < width * height) {
		uint8_t p = is.get();
		entry e = table[p];
		b = e[0];
		g = e[1];
		r = e[2];
		im[read_pixels++] = { b, g, r };
		if (read_pixels % width == 0) {
			dummy = 0;
			is.read(reinterpret_cast<char*>(&dummy), pad);
			assert(dummy == 0);
		}
	}
	return im;
}

void write_pam(std::ostream& os, std::vector<bgr>& im, int32_t width, int32_t height) {
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n", width, height);
	for (size_t row = height; row-- > 0;) {
		for (size_t c = 0; c < width; c++) {
			bgr pixel = im[row * width + c];
			uint8_t r = pixel[2];
			uint8_t g = pixel[1];
			uint8_t b = pixel[0];
			os.put(r);
			os.put(g);
			os.put(b);
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	std::ofstream os(argv[2], std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}

	if (!read_file_header(is)) {
		return EXIT_FAILURE;
	}
	uint16_t bpp = 0;
	int32_t width = 0, height = 0;
	uint32_t num_colors = 0;
	if (!read_info_header(is, bpp, width, height, num_colors)) {
		return EXIT_FAILURE;
	}
	std::vector<entry> color_table = read_color_table(is, num_colors);
	std::vector<bgr> im = read_image(is, bpp, width, height, color_table);
	write_pam(os, im, width, height);

	return EXIT_SUCCESS;
}