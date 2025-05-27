#include "ppm.h"

extern std::string JSON(const std::string& filename);

int main() {
	std::string filename("facolta.ppm");
	std::string res = JSON(filename);
	
	return EXIT_SUCCESS;
}