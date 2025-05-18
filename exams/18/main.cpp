#include <cstdint>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <bit>
#include <cassert>
#include <cmath>
#include <print>
#include <ostream>

using rgbe = std::array<uint8_t, 4>;
using rgbfloat = std::array<float, 3>;
using rgb = std::array<uint8_t, 3>;

struct image {
	size_t rows_;
	size_t cols_;
	std::vector<rgb> data_;

	image(size_t rows, size_t cols):rows_(rows), cols_(cols), data_(rows * cols) {}

	rgb& operator[](size_t i) {
		return data_[i];
	}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rawsize() {
		return rows_ * cols_ * sizeof(rgb);
	}
};
bool read_hdr_header(std::istream& is) {
	std::string token;
	is >> token;
	if (token != "#?RADIANCE") {
		return false;
	}
	while (!token.contains("FORMAT")) {
		is >> token;
	}
	if (!token.contains("32-bit_rle_rgbe")) {
		return false;
	}
	bool endhdr = false;
	while (!endhdr) {
		char c = is.get();
		if (c == '\n' && is.peek() == '\n') {
			is.get();
			endhdr = true;
		}
	}
	return true;
}

bool read_resolution_string(std::istream& is, size_t& rows, size_t& cols) {
	std::string token;
	is >> token;
	if (token != "-Y") {
		return false;
	}
	is >> rows;
	is >> token;
	if (token != "+X") {
		return false;
	}
	is >> cols;
	is.get(); // \n
	return true;
}

void read_scanline(std::ifstream& is, uint16_t columns, float& global_min, float& global_max, std::vector<rgbfloat>& float_image) {
	std::vector<rgbe> colors(columns);
	size_t index = 0;
	while (index < columns) {
		uint8_t l = is.get();
		if (l <= 127) {
			for (size_t i = 0; i < l; i++) {
				assert(index < columns);
				colors[index++][0] = is.get();
			}
		}
		else {
			uint8_t b = is.get();
			for (size_t i = 0; i < l - 128; i++) {
				assert(index < columns);
				colors[index++][0] = b;
			}
		}
	}
	index = 0;
	while (index < columns) {
		uint8_t l = is.get();
		if (l <= 127) {
			for (size_t i = 0; i < l; i++) {
				assert(index < columns);
				colors[index++][1] = is.get();
			}
		}
		else {
			uint8_t b = is.get();
			for (size_t i = 0; i < l - 128; i++) {
				assert(index < columns);
				colors[index++][1] = b;
			}
		}
	}
	index = 0;
	while (index < columns) {
		uint8_t l = is.get();
		if (l <= 127) {
			for (size_t i = 0; i < l; i++) {
				assert(index < columns);
				colors[index++][2] = is.get();
			}
		}
		else {
			uint8_t b = is.get();
			for (size_t i = 0; i < l - 128; i++) {
				assert(index < columns);
				colors[index++][2] = b;
			}
		}
	}
	index = 0;
	while (index < columns) {
		uint8_t l = is.get();
		if (l <= 127) {
			for (size_t i = 0; i < l; i++) {
				assert(index < columns);
				colors[index++][3] = is.get();
			}
		}
		else {
			uint8_t b = is.get();
			for (size_t i = 0; i < l - 128; i++) {
				assert(index < columns);
				colors[index++][3] = b;
			}
		}
	}

	for (size_t i = 0; i < columns; i++) {
		rgbe c = colors[i];
		uint8_t r = c[0];
		uint8_t g = c[1];
		uint8_t b = c[2];
		uint8_t e = c[3];
		float r_float = ((r + 0.5) / 256) * pow(2, e - 128);
		float g_float = ((g + 0.5) / 256) * pow(2, e - 128);
		float b_float = ((b + 0.5) / 256) * pow(2, e - 128);
		float_image.push_back({ r_float, g_float, b_float });
		if (r_float < global_min) {
			global_min = r_float;
		}
		if (g_float < global_min) {
			global_min = g_float;
		}
		if (b_float < global_min) {
			global_min = b_float;
		}
		if (r_float > global_max) {
			global_max = r_float;
		}
		if (g_float > global_max) {
			global_max = g_float;
		}
		if (b_float > global_max) {
			global_max = b_float;
		}
	}
}
void write_image(std::ostream& os, image& im) {
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n", im.cols_, im.rows_);
	os.write(im.rawdata(), im.rawsize());
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
	if (!read_hdr_header(is)) {
		return EXIT_FAILURE;
	}

	size_t rows = 0, cols = 0;
	if (!read_resolution_string(is, rows, cols)) {
		return EXIT_FAILURE;
	}
	image im(rows, cols);
	size_t i = 0;
	float global_min = 1000000.0, global_max = -1.0;
	std::vector<rgbfloat> float_image;
	while (i < rows) {
		uint8_t b = is.get();
		if (b == 2 && is.peek() == 2) {
			is.get();
			uint16_t columns;
			is.read(reinterpret_cast<char*>(&columns), 2);
			columns = std::byteswap(columns);
			read_scanline(is, columns, global_min, global_max, float_image);
			i++;
		}
	}

	for (size_t i = 0; i < rows * cols; i++) {
		rgbfloat f = float_image[i];
		float r_float = f[0];
		float g_float = f[1];
		float b_float = f[2];
		uint8_t r = 255 * pow((r_float - global_min) / (global_max - global_min), 0.45);
		uint8_t g = 255 * pow((g_float - global_min) / (global_max - global_min), 0.45);
		uint8_t b = 255 * pow((b_float - global_min) / (global_max - global_min), 0.45);
		im[i] = { r, g, b };
	}

	write_image(os, im);
	return EXIT_SUCCESS;
}