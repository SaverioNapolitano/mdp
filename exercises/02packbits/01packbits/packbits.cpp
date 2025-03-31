#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

int main(int argc, char** argv) {
	using namespace std::string_literals;
	if (argc != 4 || (argv[1] != "c"s && argv[1] != "d"s)) {
		return EXIT_FAILURE;
	}

	if (argv[1] == "c"s) {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}
		uint8_t c;
		uint8_t L = 0;
		std::vector<uint8_t> bytes;
		while (is.read(reinterpret_cast<char *>(&c), sizeof(c))) {
			bytes.push_back(c);
			if (bytes.size() > 1) {
				if (bytes[0] == bytes[1]) { // run
					if (bytes[0] == bytes[bytes.size() - 1]) { // keep run
						L = static_cast<uint8_t>(257 - bytes.size());
						if (L == 129) {
							os.put(L);
							os.put(bytes[0]);
							bytes.clear();
						}
					}
					else { // end run
						L = static_cast<uint8_t>(257 - (bytes.size()-1));
						os.put(L);
						os.put(bytes[0]);
						bytes.erase(begin(bytes), end(bytes) - 1);
					}
				}
				else { // copy
					if (bytes[bytes.size() - 1] != bytes[bytes.size() - 2]) { // keep copy
						L = static_cast<uint8_t>(bytes.size() - 1);
						if (L == 127) {
							if (is.peek() == bytes[bytes.size() - 1]) {
								L--;
								os.put(L);
								for (size_t i = 0; i < bytes.size() - 1; i++) {
									os.put(bytes[i]);
								}
								bytes.erase(begin(bytes), end(bytes) - 1);
							}
							else {
								os.put(L);

								for (const auto& b : bytes) {
									os.put(b);
								}

								bytes.clear();
							}
							
						}
					}
					else { // end copy
						L = static_cast<uint8_t>(bytes.size() - 3);				
						os.put(L);
						for (size_t i = 0; i < bytes.size() - 2; i++) {
							os.put(bytes[i]);
						}
						bytes.erase(begin(bytes), end(bytes) - 2);
					}
				}
			}
			
		}
		if (!bytes.empty()) {
			if (bytes[0] == bytes[1]) {
				L = static_cast < uint8_t>(257 - bytes.size());
				os.put(L);
				os.put(bytes[bytes.size() - 1]);
				bytes.clear();
			}
			else {
				L = static_cast <uint8_t>(bytes.size() - 1);
				os.put(L);
				for (const auto& b : bytes) {
					os.put(b);
				}
				bytes.clear();
			}
		}
		os.put(128);
	}
	else {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		std::ofstream os(argv[3]/*, std::ios::binary*/);
		if (!os) {
			return EXIT_FAILURE;
		}
		char c;
		while (is.get(c)) {
			uint8_t L = c;
			if (L < 128) {
				for (size_t i = 0; i < L + 1; i++) {
					is.get(c);
					uint8_t u = c;
					os.put(u);
				}
			}
			if (L > 128) {
				is.get(c);
				uint8_t u = c;
				for (size_t i = 0; i < 257 - L; i++) {
					os.put(u);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}