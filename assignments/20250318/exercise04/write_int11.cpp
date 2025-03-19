#include <print>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println(std::cerr, "Usage {} <input file> <output file>", argv[0]);
		return 1;
	}

	std::ifstream input(argv[1]/*, std::ios::binary*/);

	if (!input) {
		std::cerr << "Error opening input file\n";
		return 1;
	}

	std::ofstream output(argv[2], std::ios::binary);

	if (!output) {
		std::cerr << "Error opening output file";
		return 1;
	}

	std::vector<int> numbers{ std::istream_iterator<int>(input), std::istream_iterator<int>() };

	int bits = 0;
	for (auto& number : numbers) {
		if (std::endian::native == std::endian::little) {
			output << std::bitset<11>{~number + (uint64_t)1};
		}
		else {
			output << std::bitset<11>{~std::byteswap(number) + (uint64_t)1};
		}
		bits += 11;
	}
	int padding = (bits / 8 + 1) * 8 - bits;
	for (int i = 0; i < padding; i++) {
		output << std::bitset<1>{(uint64_t)0};
	}

	return 0;


}