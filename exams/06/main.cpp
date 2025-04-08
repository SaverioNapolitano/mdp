#include "lz78encode.h"

int main(int argc, char** argv) {
	using namespace std;
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	lz78encode(argv[1], argv[2], 2);
	return EXIT_SUCCESS;

}