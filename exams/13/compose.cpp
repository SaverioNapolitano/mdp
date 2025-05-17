#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <print>
#include <ostream>

using rgba = std::array<uint8_t, 4>;

struct image {
	size_t rows_ = 0;
	size_t cols_ = 0;
	std::vector<rgba> data_;

	image(){}
	image(size_t height, size_t width):rows_(height), cols_(width), data_(height*width){}

	rgba& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}

	rgba& operator[](size_t i) {
		return data_[i];
	}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rawsize() {
		return rows_* cols_ * sizeof(rgba);
	}

};

void write_pam(std::ostream& os, image& im) {
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n", im.cols_, im.rows_);
	os.write(im.rawdata(), im.rawsize());
}

void read_header(std::istream& is, size_t& width, size_t& height) {
	std::string token{};
	size_t depth;
	while (is >> token && token != "ENDHDR") {
		if (token == "MAXVAL") {
			int maxval;
			is >> maxval;
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype{};
			is >> tupltype;
		}
		else if (token == "WIDTH") {
			is >> width;
		}
		else if (token == "HEIGHT") {
			is >> height;
		}
		else if (token == "DEPTH") {
			is >> depth;
		}
		else {
			std::string dummy;
			std::getline(is, dummy);
		}
	}
}
image compose(image& prev, image& next, size_t x, size_t y) {
	for (size_t row = 0; row < prev.rows_; row++) {
		for (size_t col = 0; col < prev.cols_; col++) {
			if (row >= y && col >= x && row < (next.rows_ + y) && col < (next.cols_ + x)) {
				rgba foreground_pixel = next(row - y, col - x);
				rgba background_pixel = prev(row, col);
				double a_a = foreground_pixel[3] / 255.0;
				double a_b = background_pixel[3] / 255.0;
				double a_o = a_a + a_b * (1 - a_a);
				if (a_o > 0) {
					double r_a = foreground_pixel[0];
					double r_b = background_pixel[0];
					double r_o = (r_a * a_a + r_b * a_b * (1 - a_a)) / a_o;

					double g_a = foreground_pixel[1];
					double g_b = background_pixel[1];
					double g_o = (g_a * a_a + g_b * a_b * (1 - a_a)) / a_o;

					double b_a = foreground_pixel[2];
					double b_b = background_pixel[2];
					double b_o = (b_a * a_a + b_b * a_b * (1 - a_a)) / a_o;

					uint8_t r = r_o;
					uint8_t g = g_o;
					uint8_t b = b_o;
					uint8_t a = a_o * 255;
					prev(row, col) = { r, g, b, a };
				}
			}
		}
	}
	return prev;
}

image read_pam(std::ifstream& is, image& background) {
	std::string token{};
	size_t width = 0, height = 0, depth;
	while (is >> token && token != "ENDHDR") {
		if (token == "MAXVAL") {
			int maxval;
			is >> maxval;
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype{};
			is >> tupltype;
		}
		else if (token == "WIDTH") {
			is >> width;
		}
		else if (token == "HEIGHT") {
			is >> height;
		}
		else if (token == "DEPTH") {
			is >> depth;
		}
		else {
			std::string dummy;
			std::getline(is, dummy);
		}
	}
	is.get();
	image im(height, width);
	size_t pixels = 0;
	while (pixels < height * width) {
		uint8_t r = is.get();
		uint8_t g = is.get();
		uint8_t b = is.get();
		uint8_t a = depth == 4 ? is.get() : 255;
		rgba value{ r, g, b, a };
		im[pixels++] = value;
	}
	return im;
}

void fill_image(image& im) {
	size_t pixels = 0;
	while (pixels < im.rows_ * im.cols_) {
		im[pixels++] = { 0, 0, 0, 0 };
	}
}

int main(int argc, char** argv) {
	using namespace std::string_literals;

	if (argc < 2) {
		return EXIT_FAILURE;
	}
	std::string output{ argv[1] };
	output.append(".pam");
	std::ofstream os(output, std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}
	
	size_t i = 2;
	size_t width = 0, height = 0;
	while (i < argc) {
		size_t x = 0;
		size_t y = 0;
		if (argv[i] == "-p"s) {
			i++;
			x = std::stoull(argv[i++]);
			y = std::stoull(argv[i++]);
		}
		std::string filename{ argv[i++] };
		filename.append(".pam");
		std::ifstream is(filename, std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		size_t rows, cols;
		read_header(is, cols, rows);
		width = width > cols + x ? width : cols + x;
		height = height > rows + y ? height : rows + y;
	}
	image output_img(height, width);
	fill_image(output_img);
	i = 2;
	while (i < argc) {
		size_t x = 0;
		size_t y = 0;
		if (argv[i] == "-p"s) {
			i++;
			x = std::stoull(argv[i++]);
			y = std::stoull(argv[i++]);
		}
		std::string filename{ argv[i++] };
		filename.append(".pam");
		std::ifstream is(filename, std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		image im = read_pam(is, output_img);
		output_img = compose(output_img, im, x, y);
	}

	write_pam(os, output_img);
	return EXIT_SUCCESS;
}