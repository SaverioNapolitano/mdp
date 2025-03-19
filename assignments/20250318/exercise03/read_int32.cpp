#include <print>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <iterator>
#include <algorithm>
#include <bit>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println(std::cerr, "Usage: {} <input file> <output file>", argv[0]);
		return 1;
	}

	std::ifstream input(argv[1], std::ios::binary);

	if (!input) {
		std::cerr << "Error opening input file\n";
		return 1;
	}

	std::ofstream output(argv[2]/*, std::ios::binary*/);

	if (!output) {
		std::cerr << "Error opening output file\n";
		return 1;
	}

	std::vector<std::bitset<32>> numbers{ std::istream_iterator<std::bitset<32>>(input), std::istream_iterator<std::bitset<32>>() };

	for (const auto& number : numbers) {	
		output << -1 * ((int)number.to_ulong()) << "\n";
	}

	return 0;
}