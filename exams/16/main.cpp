#include <fstream> 
#include <array>
#include <cassert>
#include <cstdint>

#define MAGIC_NUMBER 0x184C2103
#define CONSTANT_VALUE 0x4D000000

int main(int argc, char** argv) {
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

	uint32_t header = 0;
	is.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (header != MAGIC_NUMBER) {
		return EXIT_FAILURE;
	}

	uint32_t length;
	is.read(reinterpret_cast<char*>(&length), sizeof(length));

	uint32_t constant_value = 0;
	is.read(reinterpret_cast<char*>(&constant_value), sizeof(constant_value));
	if (constant_value != CONSTANT_VALUE) {
		return EXIT_FAILURE;
	}

	uint32_t block_length = 0;
	std::array<uint8_t, 65536> dictionary{};
	size_t index = 0;
	while (is.read(reinterpret_cast<char*>(&block_length), sizeof(block_length))) {
		for (size_t i = 0; i < block_length;) {
			uint8_t token = 0;
			is.read(reinterpret_cast<char*>(&token), sizeof(token));
			i++;
			assert(i <= block_length);
			size_t literal_length = token >> 4;
			size_t match_length = token & 15;
			if (literal_length == 15) {
				uint8_t b = 255;
				while (b == 255) {
					is.read(reinterpret_cast<char*>(&b), sizeof(b));
					literal_length += b;
					i++;
					assert(i <= block_length);
				}
			}
			for (size_t j = 0; j < literal_length; j++) {
				uint8_t b = 0;
				is.read(reinterpret_cast<char*>(&b), sizeof(b));
				os.put(b);
				assert(index < 65536);
				dictionary[index++] = b;
				if (index == 65536) {
					index = 0;
				}
				i++;
				assert(i <= block_length);
			}
			if (i < block_length) {
				uint16_t offset = 0;
				is.read(reinterpret_cast<char*>(&offset), sizeof(offset));
				i += 2;
				assert(i <= block_length);
				if (match_length == 15) {
					uint8_t b = 255;
					while (b == 255) {
						is.read(reinterpret_cast<char*>(&b), sizeof(b));
						match_length += b;
						i++;
						assert(i <= block_length);
					}
				}
				match_length += 4;
				for (int start = static_cast<int>(index - offset); match_length-- > 0; start++) {
					while (start < 0) {
						start += 65536;
					}
					assert(start >= 0);
					assert(start <= 65535);
					os.put(dictionary[start]);
					assert(index < 65536);
					dictionary[index++] = dictionary[start];
					if (index == 65536) {
						index = 0;
					}
					if (start == 65535) {
						start = -1;
					}
				}
			}
		}
	}

	return EXIT_SUCCESS;
}