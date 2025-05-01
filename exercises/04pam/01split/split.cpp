#include <cstdint>
#include <fstream>
#include <string>
#include <print>
#include <ostream>

int main(int argc, char** argv) {
	if (argc != 2) {
		return EXIT_FAILURE;
	}
	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	std::string file_red{ argv[1] };
	file_red.insert(file_red.size() - 4, std::string{ "_R" });
	std::ofstream os_red(file_red, std::ios::binary);
	if (!os_red) {
		return EXIT_FAILURE;
	}

	std::string file_green{ argv[1] };
	file_green.insert(file_green.size() - 4, std::string{ "_G" });
	std::ofstream os_green(file_green, std::ios::binary);
	if (!os_green) {
		return EXIT_FAILURE;
	}

	std::string file_blue{ argv[1] };
	file_blue.insert(file_blue.size() - 4, std::string{ "_B" });
	std::ofstream os_blue(file_blue, std::ios::binary);
	if (!os_blue) {
		return EXIT_FAILURE;
	}

	std::string token;
	size_t h, w;
	int maxval;
	while (is >> token && token != "ENDHDR") {
		if (token == "HEIGHT") {
			is >> h;
		}
		if (token == "WIDTH") {
			is >> w;
		}
		if (token == "MAXVAL") {
			is >> maxval;
		}
	}

	if (is.get() != '\n') {
		return EXIT_FAILURE;
	}

	std::print(os_red, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 1\nMAXVAL {}\nTUPLTYPE GRAYSCALE\nENDHDR\n", w, h, maxval);
	std::print(os_green, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 1\nMAXVAL {}\nTUPLTYPE GRAYSCALE\nENDHDR\n", w, h, maxval);
	std::print(os_blue, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 1\nMAXVAL {}\nTUPLTYPE GRAYSCALE\nENDHDR\n", w, h, maxval);

	for (size_t i = 0; i < w * h; i++) {
		uint8_t r = is.get();
		uint8_t g = is.get();
		uint8_t b = is.get();
		os_red.put(r);
		os_green.put(g);
		os_blue.put(b);
	}

	return EXIT_SUCCESS;
}