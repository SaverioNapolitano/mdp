#include "mat.h"
#include "types.h"
#include <string>

extern bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames);;

int main() {
	std::string filename = "foreman_cif.y4m";
	std::vector<mat<vec3b>> frames;
	bool res = y4m_extract_color(filename, frames);
	return EXIT_SUCCESS;
}