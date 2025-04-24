#include "pbm.h"

int main() {
	BinaryImage bimg{};

	std::string s = "im1.pbm";
	bimg.ReadFromPBM(s);

	Image im{};
	im = BinaryImageToImage(bimg);

	return EXIT_SUCCESS;
}