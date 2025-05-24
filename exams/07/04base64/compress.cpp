#include "ppm.h"

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	int L = -1;
	std::vector<uint8_t> seen;
	int last_p = -1;
	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			uint8_t p = img(r, c);
			if (p == last_p) { // run
				if (L == 0) { // thought it was a copy, instead is a run
					seen.clear();
					L = 256;
				}
				if (L >= 1 && L <= 127) { // was copying
					encoded.push_back(static_cast<uint8_t>(L - 1));
					encoded.insert(end(encoded), begin(seen), end(seen) - 1);
					seen.clear();
					L = 256;
				}
				if (L == 129) { // max run
					encoded.push_back(static_cast<uint8_t>(L));
					encoded.push_back(p);
					L = 256;
				}
				L--;
			}
			else { // copy
				if (L >= 129 && L <= 255) { // was running
					encoded.push_back(static_cast<uint8_t>(L));
					encoded.push_back(static_cast<uint8_t>(last_p));
					L = -1;
				}
				if (L == 127) { // max copy
					encoded.push_back(static_cast<uint8_t>(L));
					encoded.insert(end(encoded), begin(seen), end(seen));
					seen.clear();
					L = -1;
				}
				L++;
				seen.push_back(p);
			}
			last_p = p;
		}
	}
	if (L >= 0 && L <= 127) {
		encoded.push_back(static_cast<uint8_t>(L));
		encoded.insert(end(encoded), begin(seen), end(seen));
	}
	if (L >= 129 && L <= 255) {
		encoded.push_back(static_cast<uint8_t>(L));
		encoded.push_back(static_cast<uint8_t>(last_p));
	}
	encoded.push_back(128);
}

std::string Base64Encode(const std::vector<uint8_t>& v) {
	std::vector<char> table{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
	std::string encoded;
	for (size_t i = 0; i < v.size(); i += 3) {
		uint8_t first, second, third;
		first = v[i];
		second = i + 1 < v.size() ? v[i + 1] : 128;
		third = i + 2 < v.size() ? v[i + 2] : 128;
		uint8_t f = (first & 0xFC) >> 2;
		assert(f < 64);
		encoded.push_back(table[f]);
		uint8_t s = ((first & 0x03) << 4) | ((second & 0xF0) >> 4);
		assert(s < 64);
		encoded.push_back(table[s]);
		uint8_t t = ((second & 0x0F) << 2) | ((third & 0xC0) >> 6);
		assert(t < 64);
		encoded.push_back(table[t]);
		uint8_t l = third & 0x3F;
		assert(l < 64);
		encoded.push_back(table[l]);
	}
	return encoded;
}