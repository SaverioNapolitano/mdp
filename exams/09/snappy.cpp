#include <cstdint>
#include <fstream>
#include <vector>
#include <cassert>
#include <array>
uint32_t read_preamble(std::istream& is) {
	uint8_t msb = 1;
	uint32_t size = 0;
	int bytes_to_read = 0;
	std::vector<uint8_t> bytes{};
	while (msb != 0) {
		char c;
		is.get(c);
		uint8_t u = c;
		msb = (u >> 7);
		bytes_to_read++;
		bytes.push_back(u);
	}
	for (size_t i = bytes.size(); i-- > 0;) {
		uint8_t b = bytes[i];
		for (size_t j = 7; j-- > 0;) {
			size = (size << 1) | ((b >> j) & 1);
		}
	}

	return size;
}

enum type {
	LITERAL,
	COPY_1_BYTE_OFFSET,
	COPY_2_BYTES_OFFSET,
	COPY_4_BYTES_OFFSET

};
type interpret_byte_tag(uint8_t tag, uint8_t& msb) {
	type t;
	uint8_t lsb = tag & 3;
	switch (lsb) {
	case 0:
		t = LITERAL;
		break;
	case 1:
		t = COPY_1_BYTE_OFFSET;
		break;
	case 2:
		t = COPY_2_BYTES_OFFSET;
		break;
	default:
		t = COPY_4_BYTES_OFFSET;
	}
	msb = (tag >> 2) & 63;
	return t;
	
}
uint32_t read_length_literal(uint8_t msb, std::ifstream& is) {
	uint32_t length = 0;
	
		if (msb < 60) {
			length = msb;
		}
		if (msb == 60) {
			char c;
			is.get(c);
			uint8_t b = c;
			length = b;
		}
		if (msb == 61) {
			uint8_t b1, b2;
			b1 = b2 = 0;
			char c;
			is.get(c);
			b1 = c;
			is.get(c);
			b2 = c;
			length = b2;
			length = (length << 8) | b1;

		}
		if (msb == 62) {
			uint8_t b1, b2, b3;
			b1 = b2 = b3 = 0;
			char c;
			is.get(c);
			b1 = c;
			is.get(c);
			b2 = c;
			is.get(c);
			b3 = c;
			length = b3;
			length = (length << 8) | b2;
			length = (length << 8) | b1;
		}
		if (msb == 63) {
			uint8_t b1, b2, b3, b4;
			b1 = b2 = b3 = b4 = 0;
			char c;
			is.get(c);
			b1 = c;
			is.get(c);
			b2 = c;
			is.get(c);
			b3 = c;
			is.get(c);
			b4 = c;
			length = b4;
			length = (length << 8) | b3;
			length = (length << 8) | b2;
			length = (length << 8) | b1;
		}
	
	return ++length;
	
}


int main(int argc, char** argv) {
	using namespace std;
	if (argc != 3) {
		return EXIT_FAILURE;
	}
	ifstream is(argv[1], ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	ofstream os(argv[2], ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}
	uint32_t preamble = read_preamble(is);
	uint8_t tag = 0;
	char c;
	//vector<uint8_t> v{};
	array<uint8_t, 65536> dictionary{};
	size_t index = 0;
	while (is.get(c)) {
		tag = c;
		uint8_t msb;
		type t = interpret_byte_tag(tag, msb);
		if (t == LITERAL) {
			uint32_t length = read_length_literal(msb, is);
			uint8_t b = 0;
			for (size_t i = 0; i < length; i++) {
				is.get(c);
				b = c;
				os.put(b);
				dictionary[index++] = b;
				if (index == 65536) {
					index = 0;
				}
			}
		}
		if (t == COPY_1_BYTE_OFFSET) {
			uint32_t length = ((tag >> 2) & 7) + 4;
			uint16_t offset = (tag >> 5) & 7;
			uint8_t b;
			is.get(c);
			b = c;
			offset = (offset << 8) | b;
			assert(offset <= 2047);
			int start = index >= offset ? index - offset : 65536 + index - offset;
			for (int i = start; length-- > 0; i++) {
				while (i < 0) {
					i += 65536;
				}
				assert(i >= 0 && i < 65536);
				os.put(dictionary[i]);
				dictionary[index++] = dictionary[i];
				if (index == 65536) {
					index = 0;
				}
				if (i == 65535) {
					i = -1;
				}
			}
		}
		if (t == COPY_2_BYTES_OFFSET) {
			uint32_t length = ((tag >> 2) & 63) + 1;
			uint16_t offset = 0;
			is.get(c);
			uint8_t b1 = c;
			is.get(c);
			uint8_t b2 = c;
			offset = b2;
			offset = (offset << 8) | b1;
			assert(offset <= 65535);
			// before-the-last byte
			int start = index >= offset ? index - offset : 65536 + index - offset;
			for (int i = start; length-- > 0; i++) {
				while (i < 0) {
					i += 65536;
				}
				assert(i >= 0 && i < 65536);

				os.put(dictionary[i]);
				dictionary[index++] = dictionary[i];
				if (index == 65536) {
					index = 0;
				}
				if (i == 65535) {
					i = -1;
				}
			}
		}
		if (t == COPY_4_BYTES_OFFSET) {
			uint32_t length = ((tag >> 2) & 63) + 1;
			uint32_t offset = 0;
			is.get(c);
			uint8_t b1 = c;
			is.get(c);
			uint8_t b2 = c;
			is.get(c);
			uint8_t b3 = c;
			is.get(c);
			uint8_t b4 = c;
			offset = b4;
			offset = (offset << 8) | b3;
			offset = (offset << 8) | b2;
			offset = (offset << 8) | b1;
			// last byte
			int start = index >= offset ? index - offset : 65536 + index - offset;
			for (int i = start; length-- > 0; i++) {
				while (i < 0) {
					i += 65536;
				}
				assert(i >= 0 && i < 65536);
				os.put(dictionary[i]);
				dictionary[index++] = dictionary[i];
				if (index == 65536) {
					index = 0;
					i = -1;
				}
			}
		}
	}
	return EXIT_SUCCESS;
}