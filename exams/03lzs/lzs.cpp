#include "lzs.h"
#include <array>
#include <cassert>
#include <cstdint>
class bitreader {
	std::istream& is_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	uint32_t readbit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) :is_(is) {}
	~bitreader() {}
	std::istream& operator()(uint32_t& u, size_t n) {
		while (n-- > 0) {
			u = (u << 1) | readbit();
		}
		return is_;
	}

};

enum state {
	STARTING_POINT,
	LITERAL,
	OFFSET_OR_END_MARKER,
	OFFSET_GREATER_EQUAL_128,
	OFFSET_LOWER_128_OR_END_MARKER,
	OFFSET_LOWER_128,
	END_MARKER,
};

struct statemanager {
	state s_ = STARTING_POINT;
	statemanager(){}
	void transition(uint32_t u = 0) {
		state next_state = s_;
		if (s_ == STARTING_POINT and u == 0) {
			next_state = LITERAL;
		}
		if (s_ == STARTING_POINT and u == 1) {
			next_state = OFFSET_OR_END_MARKER;
		}
		if (s_ == LITERAL) {
			next_state = STARTING_POINT;
		}
		if (s_ == OFFSET_OR_END_MARKER and u == 0) {
			next_state = OFFSET_GREATER_EQUAL_128;
		}
		if (s_ == OFFSET_OR_END_MARKER and u == 1) {
			next_state = OFFSET_LOWER_128_OR_END_MARKER;
		}
		if (s_ == OFFSET_LOWER_128_OR_END_MARKER and u == 0) {
			next_state = END_MARKER;
		}
		if (s_ == OFFSET_LOWER_128_OR_END_MARKER and u > 0) {
			next_state = OFFSET_LOWER_128;
		}
		if (s_ == OFFSET_GREATER_EQUAL_128) {
			next_state = STARTING_POINT;
		}
		if (s_ == OFFSET_LOWER_128) {
			next_state = STARTING_POINT;
		}
		s_ = next_state;
	}

};

uint32_t read_length(bitreader& br) {
	uint32_t bits = 0;
	uint32_t length = 0;
	br(bits, 2);
	length = bits + 2;
	if (length == 2 or length == 3 or length == 4) {
		return length;
	}
	br(bits, 2);
	length = bits - 7;
	if (length == 5 or length == 6 or length == 7) {
		return length;
	}

	uint32_t N = 1;
	while (bits == 15) {
		bits = 0;
		br(bits, 4);
		N++;
	}
	// bits = length - (N*15-7); -> length = N*15-7+bits
	N--;
	length = N * 15 - 7 + bits;
	return length;
}
void lzs_decompress(std::istream& is, std::ostream& os) {
	bitreader br(is);
	uint32_t bit = 0;
	uint32_t symbol = 0;
	uint32_t offset = 0;
	statemanager manager{};
	std::array<uint8_t, 2048> dictionary{};
	size_t index = 0;
	uint32_t length = 0;

	while (manager.s_ != END_MARKER) {
		br(bit, 1);
		manager.transition(bit);
		if (manager.s_ == LITERAL) {
			br(symbol, 8);
			os.put(symbol);
			dictionary[index++] = symbol;
			symbol = 0;
			if (index == 2048) {
				index = 0;
			}
			manager.transition();
		}
		if (manager.s_ == OFFSET_GREATER_EQUAL_128) {
			br(offset, 11);
			length = read_length(br);
			//assert(index >= offset);
			size_t start = index >= offset ? index - offset: 2048 + index - offset;
			for (size_t i = start; length-- > 0; i++) {
				assert(i < 2048);
				os.put(dictionary[i]);
				dictionary[index++] = dictionary[i];
				if (index == 2048) {
					index = 0;
				}
				if (i == 2047) {
					i = -1;
				}
			}
			manager.transition();
			offset = 0;
		}
		if (manager.s_ == OFFSET_LOWER_128_OR_END_MARKER) {
			br(offset, 7);
			manager.transition(offset);
		}
		if (manager.s_ == OFFSET_LOWER_128) {
			length = read_length(br);
			//assert(index >= offset);
			size_t start = index >= offset ? index - offset : 2048 + index - offset;
			for (size_t i = start; length-- > 0; i++) {
				assert(i < 2048);
				os.put(dictionary[i]);
				dictionary[index++] = dictionary[i];
				if (index == 2048) {
					index = 0;
				}
				if (i == 2047) {
					i = -1;
				}
			}
			manager.transition();
			offset = 0;
		}
		bit = 0;
		
	}

}