#include <print>
#include <iostream>
#include <fstream>
#include <set>
#include <iterator>
#include <vector>



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

	std::ofstream  output(argv[2], std::ios::binary);
	if (!output) {
		std::cerr << "Error opening output file\n";
		return 1;
	}

	std::multiset<uint8_t> all_bytes{ std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>() };

	input.clear();
	input.seekg(0);

	std::set<uint8_t> keys{ std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>()};

	std::vector<int> values;

	for (const auto& byte : keys) {
		int occurrences = all_bytes.count(byte);
		values.push_back(occurrences);
	}

	int i = 0;
	for (const auto& byte : keys) {
		int b = (int)byte;
		output << std::hex << b << "\t" << std::dec << values[i] << "\n";
		i++;
	}

	return 0;
}