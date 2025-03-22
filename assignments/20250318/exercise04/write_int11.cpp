#include <print>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
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

	int buffer_size = 0;
	std::bitset<8> buffer;
	for (auto& num : numbers) {
		int steps = 0;
		while (steps < 11) {
			int shift = num >> 10;
			buffer[0] = (shift) & 0x1;
			num <<= 1;
			buffer_size++;
			if (buffer_size == 8) {
				output.write(reinterpret_cast<char*>(&buffer), sizeof(char));
				buffer_size = 0;
			}
			else {
				buffer <<= 1;
			}
			steps++;
		}
	}

	while (buffer_size > 0) {
		buffer_size++;
		if (buffer_size == 8) {
			output.write(reinterpret_cast<char*>(&buffer), sizeof(char));
			break;
		}
		buffer <<= 1;
	}

	return 0;
}