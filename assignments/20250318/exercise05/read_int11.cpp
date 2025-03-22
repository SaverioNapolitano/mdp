#include <print>
#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <string>
#include <iterator>

int main(int argc, char** argv) {
	if (argc != 3) {
		std::println(std::cerr, "Usage {} <input file> <output file>", argv[0]);
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
	}

	std::bitset<8> bit{};
	std::bitset<16> num{};
	int read_bits = 0;
	while (input.read(reinterpret_cast<char*>(&bit), sizeof(char))) {
		for (size_t i = 0; i < 8; i++) {
			num[10 - read_bits] = bit[7 - i];
			read_bits++;
			if (read_bits == 11) {
				for (size_t j = 11; j < 16; j++) {
					num[j] = num[10] == 1? 1: 0;
				}
				int16_t n = static_cast<int16_t>(num.to_ulong());
				output << n << '\n';
				read_bits = 0;
			}
		}
	}

	return 0;
}