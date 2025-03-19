#include <print>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <bitset>
#include <bit>

int main(int argc, char** argv) {

	if (argc != 3) {
		std::println(std::cerr, "Usage: {} <input file> <output file>", argv[0]);
		return 1;
	}

	std::ifstream input(argv[1]/*, std::ios::binary*/);

	if (!input) {
		std::cerr << "Error opening input file\n";
		return 1;
	}

	std::ofstream output(argv[2], std::ios::binary);

	if (!output) {
		std::cerr << "Error opening output file\n";
		return 1;
	}

	std::vector<int> numbers{ std::istream_iterator<int>(input), std::istream_iterator<int>() };

	for (auto& number : numbers) {
		if (std::endian::native == std::endian::little) {
			output << std::bitset<32>{~number + (uint64_t)1};
		}
		else {
			output << std::bitset<32>{~std::byteswap(number) + (uint64_t)1};
		}
		
	}

	return 0;


}