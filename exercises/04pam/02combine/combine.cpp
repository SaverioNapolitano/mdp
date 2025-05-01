#include <cstdint>
#include <fstream>
#include <print>
#include <ostream>
#include <string>

int main(int argc, char** argv) {
	if (argc != 2) {
		return EXIT_FAILURE;
	}

	std::string filename_red{ argv[1] };
	filename_red.append("_R.pam");
	std::ifstream is_red(filename_red, std::ios::binary);
	if (!is_red) {
		return EXIT_FAILURE;
	}

	std::string filename_green{ argv[1] };
	filename_green.append("_G.pam");
	std::ifstream is_green(filename_green, std::ios::binary);
	if (!is_green) {
		return EXIT_FAILURE;
	}

	std::string filename_blue{ argv[1] };
	filename_blue.append("_B.pam");
	std::ifstream is_blue(filename_blue, std::ios::binary);
	if (!is_blue) {
		return EXIT_FAILURE;
	}

	std::string filename_reconstructed{ argv[1] };
	filename_reconstructed.append("_reconstructed.pam");
	std::ofstream os(filename_reconstructed, std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}

	std::string token;
	std::string dummy;
	size_t h, w, dum;
	int maxval, d;
	while (is_red >> token && token != "ENDHDR") {
		is_green >> dummy;
		is_blue >> dummy;
		if (token == "WIDTH") {
			is_red >> w;
			is_green >> dum;
			is_blue >> dum;
		}
		if (token == "HEIGHT") {
			is_red >> h;
			is_green >> dum;
			is_blue >> dum;
		}
		if (token == "MAXVAL") {
			is_red >> maxval;
			is_green >> d;
			is_blue >> d;
		}
	}
	is_green >> dummy;
	is_blue >> dummy;
	if (is_red.get() != '\n' || is_green.get() != '\n' || is_blue.get() != '\n') {
		return EXIT_FAILURE;
	}
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 3\nMAXVAL {}\nTUPLTYPE RGB\nENDHDR\n", w, h, maxval);

	for (size_t i = 0; i < w * h; i++) {
		uint8_t r = is_red.get();
		uint8_t g = is_green.get();
		uint8_t b = is_blue.get();
		os.put(r);
		os.put(g);
		os.put(b);
	}

	return EXIT_SUCCESS;
	
}