#include <cstdint>
#include <fstream>
#include <print>
#include <ostream>
#include <vector>
#include <string>
#include <array>
#include <cassert>

struct image {
	size_t width_ = 0;
	size_t height_ = 0;
	std::vector<uint8_t> data_;

	image(){}
	image(size_t width, size_t height) :width_(width), height_(height), data_(width* height) {}

	uint8_t& operator()(size_t r, size_t c) {
		return data_[r * width_ + c];
	}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rawsize() {
		return width_ * height_ * sizeof(uint8_t);
	}
};

image read_pgm(std::istream& is) {
	std::string token;
	size_t width = 0, height = 0, maxval = 0;
	is >> token; // P5
	is.get();
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	is >> width;
	is.get();
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	is >> height;
	is.get();
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	is >> maxval;
	if (is.peek() == '#') {
		char c = ' ';
		while (c != '\n') {
			c = is.get();
		}
	}
	is.get();
	image im(width, height);
	is.read(im.rawdata(), im.rawsize());
	return im;
}

size_t pixel_level(size_t r, size_t c) {
	if (r % 8 == 0 && c % 8 == 0) {
		return 0;
	}
	if (r % 8 == 0 && c % 8 == 4) {
		return 1;
	}
	if (r % 8 == 4 && (c % 8 == 0 || c % 8 == 4)) {
		return 2;
	}
	if ((r % 8 == 0 || r % 8 == 4) && (c % 8 == 2 || c % 8 == 6)) {
		return 3;
	}
	if ((r % 8 == 2 || r % 8 == 6) && c % 2 == 0) {
		return 4;
	}
	if (r % 2 == 0 && c % 2 != 0) {
		return 5;
	}
	return 6;
}

void create_mlt(std::ostream& os, image& im) {
	os.write("MULTIRES", 8);
	os.write(reinterpret_cast<char*>(&im.width_), 4);
	os.write(reinterpret_cast<char*>(&im.height_), 4);
	std::array<std::vector<uint8_t>, 7> levels;
	for (size_t r = 0; r < im.height_; r++) {
		for (size_t c = 0; c < im.width_; c++) {
			size_t i = pixel_level(r, c);
			levels[i].push_back(im(r, c));
		}
	}
	for (const auto& level : levels) {
		os.write(reinterpret_cast<const char*>(level.data()), level.size());
	}
}

void create_pgm(std::istream& is, std::string prefix) {
	std::vector<std::string> filenames(7);
	for (size_t i = 1; i < 8; i++) {
		filenames[i - 1] = prefix;
		filenames[i - 1].append("_");
		filenames[i - 1].append(std::to_string(i));
		filenames[i - 1].append(".pgm");
	}
	
	std::string token(8, ' ');
	is.read(token.data(), 8); // MULTIRES
	size_t width = 0, height = 0;
	is.read(reinterpret_cast<char*>(&width), 4);
	is.read(reinterpret_cast<char*>(&height), 4);
	image im(width, height);

	std::array<std::vector<uint8_t>, 7> levels;
	std::array<size_t, 7> dims{};

	for (size_t r = 0; r < height; r++) {
		for (size_t c = 0; c < width; c++) {
			dims[pixel_level(r, c)]++;
		}
	}

	for (size_t i = 0; i < 7; i++) {
		levels[i].resize(dims[i]);
		is.read(reinterpret_cast<char*>(levels[i].data()), levels[i].size());
	}

	std::ofstream output7(filenames[6], std::ios::binary);
	std::array<size_t, 7> indices{};
	for (size_t r = 0; r < height; r++) {
		for (size_t c = 0; c < width; c++) {
			size_t l = pixel_level(r, c);
			im(r, c) = levels[l][indices[l]];
			indices[l]++;
		}
	}
	std::print(output7, "P5 {} {} 255\n", width, height);
	output7.write(im.rawdata(), im.rawsize());

	std::ofstream output6(filenames[5], std::ios::binary);
	for (size_t r = 1; r < height; r+=2) {
		for (size_t c = 0; c < width; c++) {
			im(r, c) = im(r - 1, c);
		}
	}
	std::print(output6, "P5 {} {} 255\n", width, height);
	output6.write(im.rawdata(), im.rawsize());

	std::ofstream output5(filenames[4], std::ios::binary);
	for (size_t r = 0; r < height; r++) {
		for (size_t c = 1; c < width; c+=2) {
			im(r, c) = im(r, c - 1);
		}
	}
	std::print(output5, "P5 {} {} 255\n", width, height);
	output5.write(im.rawdata(), im.rawsize());

	std::ofstream output4(filenames[3], std::ios::binary);
	for (size_t r = 2; r < height; r+=4) {
		for (size_t c = 0; c < width; c++) {
			im(r, c) = im(r - 1, c);
			if (r + 1 < height) {
				im(r + 1, c) = im(r - 1, c);
			}
		}
	}
	std::print(output4, "P5 {} {} 255\n", width, height);
	output4.write(im.rawdata(), im.rawsize());

	std::ofstream output3(filenames[2], std::ios::binary);
	for (size_t r = 0; r < height; r++) {
		for (size_t c = 2; c < width; c+=4) {
			im(r, c) = im(r, c - 1);
			if (c + 1 < width) {
				im(r, c + 1) = im(r, c - 1);
			}
		}
	}
	std::print(output3, "P5 {} {} 255\n", width, height);
	output3.write(im.rawdata(), im.rawsize());

	std::ofstream output2(filenames[1], std::ios::binary);
	for (size_t r = 4; r < height; r+=8) {
		for (size_t c = 0; c < width; c++) {
			im(r, c) = im(r - 1, c);
			if (r + 1 < height) {
				im(r + 1, c) = im(r - 1, c);
			}
			if (r + 2 < height) {
				im(r + 2, c) = im(r - 1, c);
			}
			if (r + 3 < height) {
				im(r + 3, c) = im(r - 1, c);
			}
		} 
	}
	std::print(output2, "P5 {} {} 255\n", width, height);
	output2.write(im.rawdata(), im.rawsize());

	std::ofstream output1(filenames[0], std::ios::binary);
	for (size_t r = 0; r < height; r++) {

		for (size_t c = 4; c < width; c++) {
			if (c % 8 != 0) {
				im(r, c) = im(r, c - 1);
			}
		}
	}
	std::print(output1, "P5 {} {} 255\n", width, height);
	output1.write(im.rawdata(), im.rawsize());
	
}

int main(int argc, char** argv) {
	if (argc != 4) {
		return EXIT_FAILURE;
	}

	if (std::string(argv[1]) == "c") {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}
		image im = read_pgm(is);
		create_mlt(os, im);
	}
	else if (std::string(argv[1]) == "d") {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		create_pgm(is, std::string(argv[3]));
	}
	else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}