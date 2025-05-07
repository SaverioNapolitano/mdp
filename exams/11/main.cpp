#include "ppm.h"
#include "image_operations.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <functional>
#include <exception>
#include <cassert>
#include <cstdint>

using namespace std;

enum type {
	INT8 = 'i',
	UINT8 = 'U',
	INT16 = 'I',
	INT32 = 'l'
};

void get_dim(std::ifstream& is, unsigned& w, unsigned& h, std::string name) {
	while (w == -1 || h == -1) {
		uint8_t marker = is.get();
		if (marker == UINT8 || marker == INT8) {
			uint8_t size = is.get();
			string axis(size, ' ');
			is.read(axis.data(), size);
			if (axis == name) {
				uint8_t m = is.get();
				if (m == INT8 || m == UINT8) {
					w = is.get();
				}
				if (m == INT16) {
					is.read(reinterpret_cast<char*>(&w), 2);
					uint8_t lsb = w & 0xFF;
					uint8_t msb = (w >> 8) & 0xFF;
					w = 0;
					w = ((w | lsb) << 8) | msb;
				}
				if (m == INT32) {
					is.read(reinterpret_cast<char*>(&w), 4);
					uint8_t lsb1 = w & 0xFF;
					uint8_t lsb2 = (w >> 8) & 0xFF;
					uint8_t msb1 = (w >> 16) & 0xFF;
					uint8_t msb2 = (w >> 24) & 0xFF;
					w = 0;
					w = ((w | lsb1) << 8) | lsb2;
					w = (w << 8) | msb1;
					w = (w << 8) | msb2;
				}
			}
			else {
				uint8_t m = is.get();
				if (m == INT8 || m == UINT8) {
					h = is.get();
				}
				if (m == INT16) {
					is.read(reinterpret_cast<char*>(&h), 2);
					uint8_t lsb = h & 0xFF;
					uint8_t msb = (h >> 8) & 0xFF;
					h = 0;
					h = ((h | lsb) << 8) | msb;
				}
				if (m == INT32) {
					is.read(reinterpret_cast<char*>(&h), 4);
					uint8_t lsb1 = h & 0xFF;
					uint8_t lsb2 = (h >> 8) & 0xFF;
					uint8_t msb1 = (h >> 16) & 0xFF;
					uint8_t msb2 = (h >> 24) & 0xFF;
					h = 0;
					h = ((h | lsb1) << 8) | lsb2;
					h = (h << 8) | msb1;
					h = (h << 8) | msb2;
				}
			}
		}
	}	
}

void read_canvas(std::ifstream& is, unsigned& w, unsigned& h, char& r, char& g, char& b) {
	bool end = false;
	while (!end) {
		uint8_t marker = is.get();
		if (marker == INT8 || marker == UINT8) {
			uint8_t count = is.get();
			string token(count, ' ');
			is.read(token.data(), count);
			if (token == "canvas"s) {
				get_dim(is, w, h, "width"s);
				is.get(); // marker 'i' for "background"
				token.clear();
				uint8_t size = is.get();
				token.resize(size);
				is.read(token.data(), size);
				if (token == "background"s) {
					is.get(); // '['
					is.get(); // '$'
					is.get(); // 'U'
					is.get(); // '#'
					while (!end) {
						uint8_t mark = is.get();
						if (mark == INT8 || mark == UINT8) {
							is.get(); // size 3, elements to read
							r = is.get();
							g = is.get();
							b = is.get();
							end = true;
						}
					}
				}
			}
		}
	}
}

uint32_t manage_size(std::ifstream& is) {
	is.get(); // '$'
	is.get(); // 'U'
	is.get(); // '#'
	uint8_t t = is.get();
	uint32_t n = 0;
	if (t == INT8 || t == UINT8) {
		n = is.get();
	}
	if (t == INT16) {
		is.read(reinterpret_cast<char*>(&n), 2);
		uint8_t lsb = n & 0xFF;
		uint8_t msb = (n >> 8) & 0xFF;
		n = 0;
		n = ((n | lsb) << 8) | msb;
	}
	if (t == INT32) {
		is.read(reinterpret_cast<char*>(&n), 4);
		uint8_t lsb1 = n & 0xFF;
		uint8_t lsb2 = (n >> 8) & 0xFF;
		uint8_t msb1 = (n >> 16) & 0xFF;
		uint8_t msb2 = (n >> 24) & 0xFF;
		n = 0;
		n = ((n | lsb1) << 8) | lsb2;
		n = (n << 8) | msb1;
		n = (n << 8) | msb2;
	}
	return n;
}

void read_image(std::ifstream& is, image<vec3b>& img, std::string sOutput) {
	unsigned x = -1;
	unsigned y = -1;
	unsigned w = -1;
	unsigned h = -1;
	bool end = false;
	while (!end && is) {
		uint8_t marker = is.get();
		if (marker == INT8 || marker == UINT8) {
			uint8_t count = is.get();
			std::string token(count, ' ');
			is.read(token.data(), count);
			if (token == "image"s) {
				get_dim(is, x, y, "x"s);
				get_dim(is, w, h, "width"s);
				end = true;
				std::string str{ "image : x,y,width,height,data," };
				cout << str << std::endl;
			}
			else if (token != "elements"s) {
				uint8_t b = 0;
				std::vector<std::string> v{};
				v.push_back(token);
				while (b != '}') {
					b = is.get();
					if (b == INT8) {
						uint8_t size = is.get();
						std::string s(size, ' ');
						is.read(s.data(), size);
						bool name = true;
						for (size_t i = 0; i < size; i++) {
							if (s[i] != 45 && s[i] < 97 && s[i] > 122) {
								name = false;
								break;
							}
						}
						if (name) {
							v.push_back(s);
							b = is.get();
							std::string dummy{};
							if (b == UINT8 || b == INT8) {
								is.get();
							}
							if (b == INT16) {
								dummy.resize(2);
								is.read(dummy.data(), 2);
							}
							if (b == INT32) {
								dummy.resize(4);
								is.read(dummy.data(), 4);
							}
							if (b == '[') {
								uint32_t n = manage_size(is);
								dummy.resize(n);
								is.read(dummy.data(), n);
							}
							dummy.clear();
						}
						s.clear();
					}
				}
				std::string printable{ v[0]};
				printable.append(" : ");
				for (size_t i = 1; i < v.size(); i++) {
					printable.append(v[i]);
					printable.append(",");
				}
				cout << printable << std::endl;
				v.clear();
				printable.clear();
			}
		}
	}
	if (is) {
		is.get(); // marker 'i' for data
		std::string data(4, ' ');
		is.get(); // size 4 for data
		is.read(data.data(), 4); // "data"
		is.get(); // '['
		uint32_t n = manage_size(is);
		int r = x;
		int c = y;
		image<vec3b> im(w, h);
		int j = 0;
		int k = 0;
		for (size_t i = 0; i < n / 3; i++) {
			char red = is.get();
			char green = is.get();
			char blue = is.get();
			vec3b v(red, green, blue);
			assert(j < im.height() and k < im.width());
			im(k++, j) = v;
			if (k >= w) {
				k = 0;
				j++;
			}
		}
		paste(img, im, x, y);
		writeP6(sOutput, im);
	}
}

int convert(const string& sInput, const string& sOutput) {

	// Dal file UBJ devo estrarre le informazioni e creare il canvas
	ifstream is(sInput, std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	ofstream os(sOutput, std::ios::binary);
	if (!os) {
		return EXIT_FAILURE;
	}

	ofstream canvas("canvas.ppm", std::ios::binary);
	if (!canvas) {
		return EXIT_FAILURE;
	}

	unsigned w = -1; // TODO : modificare
	unsigned h = -1; // TODO : modificare
	char r, g, b;
	read_canvas(is, w, h, r, g, b);
	vec3b background(r, g, b);

	//return EXIT_FAILURE; // CHECK 1 (failed)


	image<vec3b> img(w, h);

	// Per accedere ai pixel di img posso usare img(x,y) oppure img.begin() e img.end()

	for (unsigned int r = 0; r < img.height(); r++) {
		for (unsigned int c = 0; c < img.width(); c++) {
			img(c, r) = background;
		}
	}
	if (!writeP6("canvas.ppm"s, img)) {
		return EXIT_FAILURE;
	}

	//return EXIT_FAILURE; // CHECK 2

	// Dal file UBJ devo estrarre le informazioni sulle immagini da incollare su img 
	
	std::string s{ "image" };
	int i = 1;
	while (is) {
		std::string str{ s };
		str.append(std::to_string(i));
		str.append(".ppm");
		read_image(is, img, str);
		str.clear();
		i++;
	}

	// Output in formato PPM
	if (!writeP6(sOutput, img))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

	// TODO : gestire la linea di comando
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	string sInput = argv[1];
	string sOutput = argv[2];

	return convert(sInput, sOutput);
}