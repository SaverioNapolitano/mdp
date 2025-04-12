#include <cstdint>
#include <fstream>
#include <array>
#include <cassert>

#define LZVN_COMPRESSED 0x6E787662
#define END_OF_STREAM 0x24787662

enum opcode {
	EOS,
	NOP,
	LARGE_LITERAL,
	LARGE_MATCH,
	SMALL_LITERAL,
	SMALL_MATCH,
	MEDIUM_DISTANCE,
	LARGE_DISTANCE,
	PREVIOUS_DISTANCE,
	SMALL_DISTANCE,
};

opcode decode_command(uint8_t command) {
	if (command == 0b00000110) {
		return EOS;
	}
	if (command == 0b00010110 or command == 0b00001110) {
		return NOP;
	}
	if (command == 0b11100000) {
		return LARGE_LITERAL;
	}
	if (command == 0b11110000) {
		return LARGE_MATCH;
	}
	if ((command >> 4) == 0b1110) {
		return SMALL_LITERAL;
	}
	if ((command >> 4) == 0b1111) {
		return SMALL_MATCH;
	}
	if ((command >> 5) == 0b101) {
		return MEDIUM_DISTANCE;
	}
	if ((command & 7) == 0b111) {
		return LARGE_DISTANCE;
	}
	if ((command & 7) == 0b110) {
		return PREVIOUS_DISTANCE;
	}
	return SMALL_DISTANCE;
}

void update_dictionary_and_output(std::istream& is, std::ostream& os, std::array<uint8_t, UINT16_MAX + 1>& dictionary,
	size_t& index, uint16_t literal_length, uint16_t match_length, uint16_t match_distance) {
	for (uint16_t i = 0; i < literal_length; i++) {
		uint8_t literal = 0;
		is.read(reinterpret_cast<char*>(&literal), sizeof(literal));
		os.put(literal);
		dictionary[index++] = literal;
		if (index == UINT16_MAX + 1) {
			index = 0;
		}
	}

	for (int start = static_cast<int>(index - match_distance); match_length-- > 0; start++) {
		while (start < 0) {
			start += UINT16_MAX + 1;
		}
		assert(start < UINT16_MAX + 1);
		os.put(dictionary[start]);
		dictionary[index++] = dictionary[start];
		if (index == UINT16_MAX + 1) {
			index = 0;
		}
		if (start == UINT16_MAX) {
			start = -1;
		}
	}
}
void execute_command(uint8_t command, opcode op, std::istream& is, std::ostream& os, 
	std::array<uint8_t, UINT16_MAX + 1>& dictionary, size_t& index, uint16_t& match_distance) {
	if (op == SMALL_DISTANCE) {
		uint8_t literal_length = (command >> 6) & 3;
		assert(literal_length <= 3);
		uint8_t match_length = ((command >> 3) & 7) + 3;
		assert(match_length >= 3 and match_length <= 10);
		match_distance = command & 7;
		assert(match_distance != 6 and match_distance != 7);
		uint8_t b = 0;
		is.read(reinterpret_cast<char*>(&b), sizeof(b));
		match_distance = (match_distance << 8) | b;
		assert(match_distance <= 1535);
		update_dictionary_and_output(is, os, dictionary, index, literal_length, match_length, match_distance);
	}
	if (op == MEDIUM_DISTANCE) {
		uint8_t literal_length = (command >> 3) & 3;
		assert(literal_length <= 3);
		uint8_t match_length = command & 7;
		uint8_t b = 0;
		is.read(reinterpret_cast<char*>(&b), sizeof(b));
		match_length = ((match_length << 2) | (b & 3)) + 3;
		assert(match_length >= 3 and match_length <= 34);
		uint8_t b1 = 0;
		is.read(reinterpret_cast<char*>(&b1), sizeof(b1));
		match_distance = b1;
		match_distance = (match_distance << 6) | (b >> 2);
		assert(match_distance <= 16683);
		update_dictionary_and_output(is, os, dictionary, index, literal_length, match_length, match_distance);
	}
	if (op == LARGE_DISTANCE) {
		uint8_t literal_length = (command >> 6) & 3;
		assert(literal_length <= 3);
		uint8_t match_length = ((command >> 3) & 7) + 3;
		assert(match_length >= 3 and match_length <= 10);
		uint8_t b, b1;
		b = b1 = 0;
		is.read(reinterpret_cast<char*>(&b), sizeof(b));
		is.read(reinterpret_cast<char*>(&b1), sizeof(b1));
		match_distance = b1;
		match_distance = (match_distance << 8) | b;
		update_dictionary_and_output(is, os, dictionary, index, literal_length, match_length, match_distance);
	}
	if (op == PREVIOUS_DISTANCE) {
		uint8_t literal_length = (command >> 6) & 3;
		assert(literal_length <= 3);
		uint8_t match_length = ((command >> 3) & 7) + 3;
		assert(match_length >= 3 and match_length <= 10);
		// match distance stays the same 
		update_dictionary_and_output(is, os, dictionary, index, literal_length, match_length, match_distance);
	}
	if (op == SMALL_MATCH) {
		// no literals
		uint8_t match_length = command & 15;
		assert(match_length <= 15);
		// match distance stays the same 
		update_dictionary_and_output(is, os, dictionary, index, 0, match_length, match_distance);
	}
	if (op == LARGE_MATCH) {
		// no literals 
		uint8_t b = 0;
		is.read(reinterpret_cast<char*>(&b), sizeof(b));
		uint16_t match_length = static_cast<uint16_t>(b) + 16;
		assert(match_length >= 16 and match_length <= 271);
		// match distance stays the same 
		update_dictionary_and_output(is, os, dictionary, index, 0, match_length, match_distance);
	}
	if (op == SMALL_LITERAL) {
		uint8_t literal_length = command & 15;
		// no copy -> no match length or distance used
		// do the rest
		update_dictionary_and_output(is, os, dictionary, index, literal_length, 0, 0);
	}
	if (op == LARGE_LITERAL) {
		uint8_t b = 0;
		is.read(reinterpret_cast<char*>(&b), sizeof(b));
		uint16_t literal_length = b + 16;
		assert(literal_length >= 16 and literal_length <= 271);
		// no copy -> no match length or distance used
		// do the rest
		update_dictionary_and_output(is, os, dictionary, index, literal_length, 0, 0);
	}
}
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

	std::array<uint8_t, UINT16_MAX + 1> dictionary{};
	size_t index = 0;

	uint32_t header = 0;
	is.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (header != LZVN_COMPRESSED) {
		return EXIT_FAILURE;
	}

	uint32_t output_size = 0;
	is.read(reinterpret_cast<char*>(&output_size), sizeof(output_size));

	uint32_t block_size = 0;
	is.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));

	uint8_t command = 0;
	uint16_t match_distance = 0;
	opcode op = NOP;
	while (op != EOS) {
		is.read(reinterpret_cast<char*>(&command), sizeof(command));
		op = decode_command(command);
		execute_command(command, op, is, os, dictionary, index, match_distance);
	}

	return EXIT_SUCCESS;
}