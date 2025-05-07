#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <print>

void display_pieces(std::ifstream& is, size_t tabs) {
	std::string len{};
	char c;
	while ((c = is.get()) != ':') {
		len.append(1, c);
	}
	std::string str{};
	for (size_t i = 0; i < std::stoull(len); i++) {
		c = is.get();
		str.append(1, c);
	}
	if (str.size() % 20 != 0) {
		exit(-1);
	}
	size_t k = 1;
	std::cout << std::endl;

	for (size_t i = 0; i < str.size(); i += 20) {
		for (size_t i = 0; i < tabs; i++) {
			std::cout << '\t';
		}
		for (size_t j = i; j < k * 20; j++) {
			std::print("{:02x}", str[j]);
		}
		k++;
		std::cout << std::endl;
	}
}

void display_string(std::ifstream& is, std::string len, bool& pieces) {
	using namespace std::string_literals;
	char c;
	while ((c = is.get()) != ':') {
		len.append(1, c);
	}
	std::string str{};
	for (size_t i = 0; i < std::stoull(len); i++) {
		c = is.get();
		str.append(1, c);
	}
	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] < 32 || str[i] > 126) {
			str[i] = '.';
		}
	}
	std::cout << '"' << str << '"';
	pieces = str == "pieces"s ? true : false;
	
}

void display_int(std::ifstream& is) {
	char c;
	std::string number{};
	while ((c = is.get()) != 'e') {
		number.append(1, c);
	}
	std::cout << number << std::endl;
}

void display_dict(std::ifstream& is, size_t tabs, bool has_to_tab = true);

void display_list(std::ifstream& is, size_t tabs, bool has_to_tab = true) {
	if (has_to_tab) {
		for (size_t i = 0; i < tabs; i++) {
			std::cout << '\t';
		}
	}
	std::cout << '[' << std::endl;
	char c;
	bool pieces = false;
	while ((c = is.get()) != 'e') {
		if (c == 'i') {
			for (size_t i = 0; i < tabs + 1; i++) {
				std::cout << '\t';
			}
			display_int(is);
		}
		if (c == 'l') {
			display_list(is, tabs + 1);
		}
		if (c == 'd') {
			display_dict(is, tabs + 1);
		}
		if (c >= 48 && c <= 57) {
			for (size_t i = 0; i < tabs + 1; i++) {
				std::cout << '\t';
			}
			std::string len{ c };
			display_string(is, len, pieces);
			std::cout << std::endl;
		}
	}
	for (size_t i = 0; i < tabs; i++) {
		std::cout << '\t';
	}
	std::cout << ']' << std::endl;
}

void display_dict(std::ifstream& is, size_t tabs, bool has_to_tab) {
	if (has_to_tab) {
		for (size_t i = 0; i < tabs; i++) {
			std::cout << '\t';
		}
	}
	std::cout << '{' << std::endl;
	char c;
	size_t i = 0;
	bool pieces = false;
	while ((c = is.get()) != 'e') {
		if (c >= 48 && c <= 57) {
			if (i % 2 == 0) { // key
				for (size_t i = 0; i < tabs + 1; i++) {
					std::cout << '\t';
				}
			}
			std::string len{ c };
			display_string(is, len, pieces);
			if (i % 2 == 0) { // key
				std::cout << " => ";
			}
			else {
				std::cout << std::endl;
			}
			if (pieces) {
				display_pieces(is, tabs + 2);
			}
		}
		if (c == 'i') {
			display_int(is);
		}
		if (c == 'l') {
			display_list(is, tabs + 1, false);
		}
		if (c == 'd') {
			display_dict(is, tabs + 1, false);
		}
		i++;
	}
	for (size_t i = 0; i < tabs; i++) {
		std::cout << '\t';
	}
	std::cout << '}' << std::endl;
}


int main(int argc, char** argv) {
	if (argc != 2) {
		return -1;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return -1;
	}

	while (is) {
		char c = is.get();
		if (c == 'd') {
			display_dict(is, 0);
		}
		if (c == 'i') {
			display_int(is);
		}
		if (c == 'l') {
			display_list(is, 0);
		}
		if (c >= 48 && c <= 57) {
			std::string len{ c };
			bool pieces = false;
			display_string(is, len, pieces);
		}
	}

	return EXIT_SUCCESS;;


}