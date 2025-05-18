#include <cstdint>
#include <fstream>
#include <vector>
#include <algorithm>
#include <bit>
#include <print>
#include <ostream>
#include <array>
#include <cmath>

using rgb = std::array<uint8_t, 3>;

template<typename T>
struct image {
	size_t width_;
	size_t height_;
	std::vector<T> data_;

	image(size_t width, size_t height):width_(width),height_(height),data_(width*height){}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}
	size_t rawsize() {
		return width_ * height_ * sizeof(T);
	}

	T& operator()(size_t r, size_t c) {
		return data_[r * width_ + c];
	}
};

image<uint8_t> load_pgm(std::istream& is) {
	std::string token;
	is >> token;
	if (token != "P5") {
		return image<uint8_t>(0, 0);
	}
	size_t width = 0, height = 0;
	is >> width;
	is >> height;
	if (height == 0 || width == 0) {
		return image<uint8_t>(0, 0);
	}
	size_t maxval;
	is >> maxval;
	if (maxval != 65535) {
		return image<uint8_t>(0, 0);
	}
	if (is.get() != '\n') {
		return image<uint8_t>(0, 0);
	}
	image<uint16_t> im(width, height);
	is.read(im.rawdata(), im.rawsize());
	std::for_each(begin(im.data_), end(im.data_), [](uint16_t& p) {p = std::byteswap(p); p /= 256; });
	image<uint8_t> img(width, height);
	std::copy(begin(im.data_), end(im.data_), begin(img.data_));
	return img;
}

template<typename T>
void save_pgm(std::ostream& os, image<T>& im) {
	std::print(os, "P5\n{} {}\n255\n", im.width_, im.height_);
	os.write(im.rawdata(), im.rawsize());
}

bool is_red(size_t row, size_t col) {
	return row % 2 == 0 && col % 2 == 0;
}
bool is_green(size_t row, size_t col) {
	return (row % 2 == 0 && col % 2 != 0) || (row % 2 != 0 && col % 2 == 0);
}
bool is_blue(size_t row, size_t col) {
	return row % 2 != 0 && col % 2 != 0;
}

image<rgb> create_ppm(image<uint8_t>& im) {
	image<rgb> img(im.width_, im.height_);
	for (size_t row = 0; row < img.height_; row++) {
		for (size_t col = 0; col < img.width_; col++) {
			uint8_t value = im(row, col);
			rgb pixel{ 0, 0, 0 };
			if (is_red(row, col)) {
				pixel[0] = value;
			}
			if (is_green(row, col)) {
				pixel[1] = value;
			}
			if (is_blue(row, col)) {
				pixel[2] = value;
			}
			img(row, col) = pixel;
		}
	}
	return img;
}

template<typename T>
void save_ppm(std::ostream& os, image<T>& im) {
	std::print(os, "P6\n{} {}\n255\n", im.width_, im.height_);
	os.write(im.rawdata(), im.rawsize());
}

void reconstruct_green(image<rgb>& im) {
	for (int row = 0; row < im.height_; row++) {
		for (int col = 0; col < im.width_; col++) {
			rgb pixel = im(row, col);
			int pos = -1;
			if (is_red(row, col)) {
				pos = 0;
			}
			if (is_blue(row, col)) {
				pos = 2;
			}
			if (pos >= 0) {
				uint8_t x5 = pixel[pos];
				uint8_t g4 = col - 1 >= 0 ? im(row, col - 1)[1] : 0;
				uint8_t g6 = col + 1 < im.width_ ? im(row, col + 1)[1] : 0;
				uint8_t x3 = col - 2 >= 0 ? im(row, col - 2)[pos] : 0;
				uint8_t x7 = col + 2 < im.width_ ? im(row, col + 2)[pos] : 0;

				uint8_t g2 = row - 1 >= 0 ? im(row - 1, col)[1] : 0;
				uint8_t g8 = row + 1 < im.height_ ? im(row + 1, col)[1] : 0;
				uint8_t x1 = row - 2 >= 0 ? im(row - 2, col)[pos] : 0;
				uint8_t x9 = row + 2 < im.height_ ? im(row + 2, col)[pos] : 0;

				int16_t dh = abs(g4 - g6) + abs(x5 - x3 + x5 - x7);
				int16_t dv = abs(g2 - g8) + abs(x5 - x1 + x5 - x9);

				int16_t g5 = 0;
				if (dh < dv) {
					g5 = (g4 + g6) / 2 + (x5 - x3 + x5 - x7) / 4;
				}
				else if (dh > dv) {
					g5 = (g2 + g8) / 2 + (x5 - x1 + x5 - x9) / 4;
				}
				else {
					g5 = (g2 + g4 + g6 + g8) / 4 + (x5 - x1 + x5 - x3 + x5 - x7 + x5 - x9) / 8;
				}
				uint8_t g = 0;
				if (g5 > 255) {
					g = 255;
				}
				else if (g5 > 0) {
					g = static_cast<uint8_t>(g5);
				}
				im(row, col)[1] = g;
			}
		}
	}
}

void reconstruct_red_blue(image<rgb>& im) {
	for (int row = 0; row < im.height_; row++) {
		for (int col = 0; col < im.width_; col++) {
			rgb pixel = im(row, col);
			if (is_green(row, col)) {
				bool vert_red = false;
				rgb top_pixel = row - 1 >= 0 ? im(row - 1, col) : rgb{ 0, 0, 0};
				rgb bottom_pixel = row + 1 < im.height_ ? im(row + 1, col) : rgb{ 0, 0, 0 };
				rgb left_pixel = col - 1 >= 0 ? im(row, col - 1) : rgb{ 0, 0, 0 };
				rgb right_pixel = col + 1 < im.width_ ? im(row, col + 1) : rgb{ 0, 0, 0 };
				if (row - 1 >= 0) {
					vert_red = is_red(row - 1, col);
				}
				else if (row + 1 < im.width_) {
					vert_red = is_red(row + 1, col);
				}
				uint8_t r, b;
				r = vert_red ? (top_pixel[0] + bottom_pixel[0]) / 2 : (left_pixel[0] + right_pixel[0]) / 2;
				b = vert_red ? (left_pixel[2] + right_pixel[2]) / 2 : (top_pixel[2] + bottom_pixel[2]) / 2;
				im(row, col)[0] = r;
				im(row, col)[2] = b;
			}
			else {
				rgb one = row - 1 >= 0 && col - 1 >= 0 ? im(row - 1, col - 1) : rgb{ 0, 0, 0 };
				rgb two = row - 1 >= 0 ? im(row - 1, col) : rgb{ 0, 0, 0 };
				rgb three = row - 1 >= 0 && col + 1 < im.width_ ? im(row - 1, col + 1) : rgb{ 0, 0, 0 };
				rgb four = col - 1 >= 0 ? im(row, col - 1) : rgb{ 0, 0, 0 };
				rgb six = col + 1 < im.width_ ? im(row, col + 1) : rgb{ 0, 0, 0 };
				rgb seven = row + 1 < im.height_ && col - 1 >= 0 ? im(row + 1, col - 1) : rgb{ 0, 0, 0 };
				rgb eight = row + 1 < im.height_ ? im(row + 1, col) : rgb{ 0, 0, 0 };
				rgb nine = row + 1 < im.height_ && col + 1 < im.width_ ? im(row + 1, col + 1) : rgb{ 0, 0, 0 };
				
				size_t pos = is_red(row, col) ? 2 : 0;
				int16_t dn = abs(one[pos] - nine[pos]) + abs(pixel[1] - one[1] + pixel[1] - nine[1]);
				int16_t dp = abs(three[pos] - seven[pos]) + abs(pixel[1] - three[1] + pixel[1] - seven[1]);

				int16_t x5 = 0;
				if (dn < dp) {
					x5 = (one[pos] + nine[pos]) / 2 + (pixel[1] - one[1] + pixel[1] - nine[1]) / 4;
				}
				else if (dn > dp) {
					x5 = (three[pos] + seven[pos]) / 2 + (pixel[1] - three[1] + pixel[1] - seven[1]) / 4;
				}
				else {
					x5 = (one[pos] + three[pos] + seven[pos] + nine[pos]) / 4 + (pixel[1] - one[1] + pixel[1] - three[1] + pixel[1] - seven[1] + pixel[1] - nine[1]) / 8;
				}
				uint8_t x = 0;
				if (x5 > 255) {
					x = 255;
				}
				else if (x5 > 0) {
					x = static_cast<uint8_t>(x5);
				}
				im(row, col)[pos] = x;
			}
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
	image<uint8_t> im = load_pgm(is);
	if (im.height_ == 0 || im.width_ == 0) {
		return EXIT_FAILURE;
	}
	std::string output_gray(argv[2]);
	output_gray.append("_gray.pgm");
	std::ofstream gray(output_gray, std::ios::binary);
	if (!gray) {
		return EXIT_FAILURE;
	}
	save_pgm(gray, im);
	image<rgb> img = create_ppm(im);
	std::string output_bayer(argv[2]);
	output_bayer.append("_bayer.ppm");
	std::ofstream bayer(output_bayer, std::ios::binary);
	if (!bayer) {
		return EXIT_FAILURE;
	}
	save_ppm(bayer, img);
	reconstruct_green(img);
	std::string output_green(argv[2]);
	output_green.append("_green.ppm");
	std::ofstream green(output_green, std::ios::binary);
	if (!green) {
		return EXIT_FAILURE;
	}
	save_ppm(green, img);
	reconstruct_red_blue(img);
	std::string output_interp(argv[2]);
	output_interp.append("_interp.ppm");
	std::ofstream interp(output_interp, std::ios::binary);
	save_ppm(interp, img);

	return EXIT_SUCCESS;
}