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