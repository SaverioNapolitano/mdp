#include "base64.h"
#include <array>

enum table {
	CAPITAL_LETTER,
	LOWERCASE_LETTER,
	DIGIT,
	PLUS,
	SLASH,
	PAD,
	IGNORE
};

table find_entry(uint8_t u) {
	if (u >= 65 and u <= 90) {
		return CAPITAL_LETTER;
	}
	if (u >= 97 and u <= 122) {
		return LOWERCASE_LETTER;
	}
	if (u >= 48 and u <= 57) {
		return DIGIT;
	}
	if (u == 43) {
		return PLUS;
	}
	if (u == 47) {
		return SLASH;
	}
	if (u == 61) {
		return PAD;
	}
	return IGNORE;
}

uint8_t find_value(table entry, uint8_t u) {
	if (entry == CAPITAL_LETTER) {
		return u - 65;
	}
	if (entry == LOWERCASE_LETTER) {
		return u - 71;
	}
	if (entry == DIGIT) {
		return u + 4;
	}
	if (entry == PLUS) {
		return 62;
	}
	if (entry == SLASH) {
		return 63;
	}
	if (entry == PAD) {
		return 0;
	}
	return 65;
}

std::string base64_decode(const std::string& input) {
	std::string output{};
	if (input.empty()) {
		return output;
	}
	std::array<uint8_t, 3> buffer{};
	size_t i = 0;
	uint8_t b = 0;
	char c = 0;
	for (size_t j = 0; j < input.size(); j++) {
		uint8_t u = input[j];
		table entry = find_entry(u);
		if (entry == PAD) {
			u = 0;
			break;
		}
		u = find_value(entry, u);
		if (u > 64) {
			continue;
		}
		if (i == 0) {
			b = u << 2;
			buffer[i++] = b;
		}
		else {
			if (i == 1) {
				b = u >> 4;
				buffer[i - 1] |= b;
				b = u << 4;
				buffer[i++] = b;
				c = buffer[i - 2];
				output.append(1, c);
			}
			else {
				if (i == 2) {
					b = u >> 2;
					buffer[i - 1] |= b;
					b = u << 6;
					buffer[i++] = b;
					c = buffer[i - 2];
					output.append(1, c);
				}
				else {
					if (i == 3) {
						buffer[i - 1] |= u;
						c = buffer[i - 1];
						output.append(1, c);
						i = 0;
					}
				}
			}
		}
	}
	

	return output;

}