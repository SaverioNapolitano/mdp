#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

int get_metadata(std::istream& is, std::ostream& os, char* preamble, size_t size) {

	char c;
	bool comment = false;
	std::string s{};
	while (is.get(c)) {
		if (c == '#') {
			comment = true;
		}
		if (c == '\n') {
			break;
		}
		s.append(1, c);
	}
	if (comment) {
		return -1;
	}
	os.write(preamble, size - 1);
	os.write(s.c_str(), s.size());
	os.put('\n');
	return static_cast<int>(std::stoul(s));
}
int main(int argc, char** argv) {
	using namespace std::string_literals;

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
	char output_identifier[4] = "P7\n";
	char input_identifier[4];
	is.read(input_identifier, 3);
	input_identifier[3] = 0;
	if (std::string(input_identifier) != std::string(output_identifier)) {
		return EXIT_FAILURE;
	}
	os.write(output_identifier, 3);

	char input_width[7];
	is.read(input_width, 6);
	input_width[6] = 0;
	int width = -1;
	while (width < 0) {
		width = get_metadata(is, os, input_width, 7);
	}
	
	char input_height[8];
	is.read(input_height, 7);
	input_height[7] = 0;
	int height = -1;
	while (height < 0) {
		height = get_metadata(is, os, input_height, 8);
	}

	char input_depth[7];
	is.read(input_depth, 6);
	input_depth[6] = 0;
	int depth = -1;
	while (depth < 0) {
		depth = get_metadata(is, os, input_depth, 7);
	}
	if (depth != 1) {
		return EXIT_FAILURE;
	}

	char input_maxval[8];
	is.read(input_maxval, 7);
	input_maxval[7] = 0;
	int maxval = -1;
	while (maxval < 0) {
		maxval = get_metadata(is, os, input_maxval, 8);
	}

	char input_tupltype[10];
	is.read(input_tupltype, 9);
	input_tupltype[9] = 0;
	os.write(input_tupltype, 9);
	std::string s{};
	char c;
	while (is.get(c)) {
		if (c == '\n') {
			break;
		}
		s.append(1, c);
	}
	if (s != "GRAYSCALE"s) {
		return EXIT_FAILURE;
	}
	char grayscale[11] = "GRAYSCALE\n";
	os.write(grayscale, 10);
	char endhdr[8];
	is.read(endhdr, 7);
	endhdr[7] = 0;
	os.write(endhdr, 7);

	
	std::vector<std::vector<uint8_t>> image{};

	for (size_t i = 0; i < static_cast<size_t>(height); i++) {
		std::vector<uint8_t> row{};
		for (size_t j = 0; j < static_cast<size_t>(width); j++) {
			is.get(c);
			uint8_t b = c;
			row.push_back(b);
		}
		image.push_back(row);
	}

	for (size_t i = image.size(); i-- > 0;) {
		std::vector<uint8_t> row = image[i];
		for (size_t j = 0; j < row.size(); j++) {
			os.put(row[j]);
		}
	}

	return EXIT_SUCCESS;
}