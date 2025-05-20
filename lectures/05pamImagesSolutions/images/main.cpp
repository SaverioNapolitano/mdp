#include <vector>
#include <string_view> // non-owning (doesn't add cost of copying string), lightweight container for strings
#include <fstream>
#include <print>
#include <ostream> // needed to print on an output stream
#include <string>
#include <expected>
#include <array>

using rgb = std::array<uint8_t, 3>; // if we make a struct of R, G, B we don't know its size  

// flaws:
// - we have something that manages both data and access to it
// - type is defined at compile time 

// if we want something unified and at run time it will be slower 

template<typename T>
struct mat { 
	size_t rows_, cols_;
	std::vector<T> data_; // vector of vectors makes sense only if lines can have different sizes

	mat(size_t rows = 0, size_t cols = 0) : rows_(rows), cols_(cols), data_(rows*cols){}
	auto rows() const { return rows_; }
	auto cols() const { return cols_; }
	auto size() const { return data_.size(); }

	
	
	template<typename Self> // Deducing this
	auto&& operator()(this Self&& self, size_t r, size_t c) {
		return self.data_[r * self.cols_ + c];
	}
	
	auto rawdata() {
		return reinterpret_cast<char *>(data_.data());
	}

	auto rawdata() const {
		return reinterpret_cast<const char*>(data_.data());
	}

	auto rawsize() const { return size() * sizeof(T); } // size in bytes
};

bool PAMwrite(const std::string& filename, const mat<uint8_t>& img) {
	std::ofstream os(filename.data(), std::ios::binary);
	if (!os) {
		return false;
	}
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 1\nMAXVAL 255\nTUPLTYPE GRAYSCALE\nENDHDR\n", img.cols(), img.rows());
	os.write(img.rawdata(), img.rawsize());

}

bool PAMwrite(const std::string& filename, const mat<rgb>& img) {
	std::ofstream os(filename.data(), std::ios::binary);
	if (!os) {
		return false;
	}
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 3\nMAXVAL 255\nTUPLTYPE GRAYSCALE\nENDHDR\n", img.cols(), img.rows());
	os.write(img.rawdata(), img.rawsize());

}

std::expected<mat<uint8_t>, std::string> PAMread(const std::string& filename) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return std::unexpected("Cannot open file");
	}

	std::string header;
	if (!std::getline(is, header) || header != "P7") { // getline reads and resizes the string for us and returns the stream
		return std::unexpected("Wrong header");
	}
	std::string token;
	size_t w = -1, h = -1;
	while (is >> token && token != "ENDHDR") {
		if (token == "WIDTH") {
			is >> w;
		}
		else if (token == "HEIGHT") {
			is >> h;
		}
		else if (token == "DEPTH") {
			int depth;
			is >> depth;
			if (depth != 1) {
				return std::unexpected("Wrong DEPTH");
			}
		}
		else if (token == "MAXVAL") {
			int maxval;
			is >> maxval;
			if (maxval != 255) {
				return std::unexpected("Wrong MAXVAL");
			}
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype;
			is >> tupltype;
			if (tupltype != "GRAYSCALE") {
				return std::unexpected("Wrong TUPLTYPE");
			}
		}
		else {
			std::string dummy;
			std::getline(is, dummy);
		}
	} 
	// when we exit the loop we haven't read the '\n' after the ENDHDR yet, if we start filling the image we will have the first column which is wrong
	if (is.get() != '\n') {
		return std::unexpected("Wrong ENDHDR");
	}
	if (w == -1 || h == -1) {
		return std::unexpected("Missing width or height");
	}
	mat<uint8_t> img(h, w);
	is.read(img.rawdata(), img.rawsize());
	return img;
}

std::expected<mat<rgb>, std::string> PAMread_rgb(const std::string& filename) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return std::unexpected("Cannot open file");
	}

	std::string header;
	if (!std::getline(is, header) || header != "P7") { // getline reads and resizes the string for us and returns the stream
		return std::unexpected("Wrong header");
	}
	std::string token;
	size_t w = -1, h = -1;
	while (is >> token && token != "ENDHDR") {
		if (token == "WIDTH") {
			is >> w;
		}
		else if (token == "HEIGHT") {
			is >> h;
		}
		else if (token == "DEPTH") {
			int depth;
			is >> depth;
			if (depth != 3) {
				return std::unexpected("Wrong DEPTH");
			}
		}
		else if (token == "MAXVAL") {
			int maxval;
			is >> maxval;
			if (maxval != 255) {
				return std::unexpected("Wrong MAXVAL");
			}
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype;
			is >> tupltype;
			if (tupltype != "RGB") {
				return std::unexpected("Wrong TUPLTYPE");
			}
		}
		else {
			std::string dummy;
			std::getline(is, dummy);
		}
	}
	if (is.get() != '\n') {
		return std::unexpected("WRONG ENDHDR");
	}
	if (w == -1 || h == -1) {
		return std::unexpected("Missing width or height");
	}
	mat<rgb> img;
	is.read(img.rawdata(), img.rawsize());
	return img;
}

int main(int argc, char **argv) {

	/*
	if (argc != 2) {
		return EXIT_FAILURE;
	}

	mat<uint8_t> img(256, 256);
	// Use c and r to have always clear what you are using
	for (size_t r = 0; r < img.rows(); ++r) {
		// Never copy-paste a for 
		for (size_t c = 0; c < img.cols(); ++c) {
			img(r, c) = static_cast<uint8_t>(r);
		}
	}
	if (!PAMwrite(argv[1], img)) {
		return EXIT_FAILURE;
	}
	*/

	/*
	if (argc != 3) {
		return EXIT_FAILURE;
	}
	auto res = PAMread(argv[1]);
	if(res) {
		auto& img = res.value();
		// always use r and c (not i, j or others) such that you know what you are iterating over
		for (size_t r = 0; r < img.rows() / 2; ++r) {
			// Never copy-paste a for 
			for (size_t c = 0; c < img.cols(); ++c) {
				std::swap(img(r, c), img(img.rows() - 1 - r, c));
			}
		}
		PAMwrite(argv[2], img);
	}
	else {
		std::print("{}", res.error());
	}
	*/

	if (argc != 3) {
		return EXIT_FAILURE;
	}
	auto res = PAMread(argv[1]);
	if (res) {
		auto& img = res.value();
		for (size_t r = 0; r < img.rows(); ++r) {
			// Never copy-paste a for 
			for (size_t c = 0; c < img.cols() / 2; ++c) {
				std::swap(img(r, c), img(r, img.cols() - 1 - c));
			}
		}
		PAMwrite(argv[2], img);
	}
	else {
		std::print("{}", res.error());
	}


	
	return EXIT_SUCCESS;
}