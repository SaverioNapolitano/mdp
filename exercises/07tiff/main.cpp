#include <cstdint>
#include <fstream>
#include <vector>
#include <print>
#include <ostream>

template <typename T>
struct image {
	size_t width_ = 0;
	size_t height_ = 0;
	std::vector<T> data_;

	image() {}
	image(size_t width, size_t height) : width_(width), height_(height), data_(width* height) {}

	T& operator()(size_t r, size_t c) {
		return data_[r * width_ + c];
	}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rawsize() {
		return width_ * height_ * sizeof(T);
	}


};

template<typename T>
void write_pam(image<T>& im, std::ostream& os) {
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 1\nMAXVAL 255\nTUPLTYPE GRAYSCALE\nENDHDR\n", im.width_, im.height_);
	os.write(im.rawdata(), im.rawsize());
}

bool read_header(std::istream& is, uint32_t& first_ifd_offset) {
	std::string endianness(2, ' ');
	is.read(endianness.data(), 2);
	if (endianness != "II") {
		return false;
	}
	uint16_t identifier;
	is.read(reinterpret_cast<char*>(&identifier), 2);
	if (identifier != 42) {
		return false;
	}
	is.read(reinterpret_cast<char*>(&first_ifd_offset), 4);
	return true;

}

image<uint8_t> read_image(std::istream& is) {
	uint16_t number_entries = 0;
	is.read(reinterpret_cast<char*>(&number_entries), 2);
	if (number_entries < 1) {
		return image<uint8_t>();
	}
	size_t height = 0, width = 0;
	uint32_t image_offset = 0;
	for (size_t i = 0; i < number_entries; i++) {
		uint16_t tag;
		is.read(reinterpret_cast<char*>(&tag), 2);
		uint16_t type;
		is.read(reinterpret_cast<char*>(&type), 2);
		uint32_t count;
		is.read(reinterpret_cast<char*>(&count), 4);
		uint32_t value_offset;
		is.read(reinterpret_cast<char*>(&value_offset), 4);
		if (tag == 256) {
			width = value_offset;
		}
		if (tag == 257) {
			height = value_offset;
		}
		if (tag == 258 && value_offset != 8) {
			return image<uint8_t>();
		}
		if (tag == 259 && value_offset != 1) {
			return image<uint8_t>();
		}
		if (tag == 262 && value_offset != 1) {
			return image<uint8_t>();
		}
		if (tag == 278 && value_offset != height) {
			return image<uint8_t>();
		}
		if (tag == 279 && value_offset != height * width) {
			return image<uint8_t>();
		}
		if (tag == 273) {
			image_offset = value_offset;
		}
	}
	image<uint8_t> im(width, height);
	is.seekg(image_offset);
	is.read(im.rawdata(), im.rawsize());

	return im;
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

	uint32_t first_ifd_offset = 0;
	if (!read_header(is, first_ifd_offset)) {
		return EXIT_FAILURE;
	}
	is.seekg(first_ifd_offset);
	image<uint8_t> im = read_image(is);
	write_pam(im, os);

	return EXIT_SUCCESS;
}