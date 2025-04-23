#include "base64.h"

int main() {
	std::string input{ "YWJj\n" };
	std::string decoded = base64_decode(input); // abc
	input = "MQ==\n";
	decoded = base64_decode(input); // 1
	input = "MTI=\n";
	decoded = base64_decode(input); // 12 (?)
	input = "AP8A\n";
	decoded = base64_decode(input);
	input = "//==\n";
	decoded = base64_decode(input);
	input = "c2hvcnQgdGV4dA==\n";
	decoded = base64_decode(input);
	input = "UHJldHR5IGxvbmcgdGV4dCB3aGljaCByZXF1aXJlcyBtb3JlIHRoYW4gNzYgY2hhcmFjdGVycyB0\nbyBlbmNvZGUgaXQgY29tcGxldGVseS4=\n";
	decoded = base64_decode(input);
	return EXIT_SUCCESS;
	
}