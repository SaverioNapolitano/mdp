#include "mat.h"
#include <string>

extern bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames);

int main() {
	std::string filename = "foreman_cif.y4m";
	std::vector<mat<uint8_t>> frames;
	bool res = y4m_extract_gray(filename, frames);
	return EXIT_SUCCESS;
}