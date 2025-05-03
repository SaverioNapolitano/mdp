#include "pcx.h"
#include <fstream>
#include <cassert>

bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	is.get();
	is.get();
	is.get();
	is.get(); // bitsPerPlane
	uint16_t windowXMin;
	is.read(reinterpret_cast<char*>(&windowXMin), sizeof(windowXMin));
	uint16_t windowYMin;
	is.read(reinterpret_cast<char*>(&windowYMin), sizeof(windowYMin));
	uint16_t windowXMax;
	is.read(reinterpret_cast<char*>(&windowXMax), sizeof(windowXMax));
	uint16_t windowYMax;
	is.read(reinterpret_cast<char*>(&windowYMax), sizeof(windowYMax));
	uint16_t vertDPI;
	is.read(reinterpret_cast<char*>(&vertDPI), sizeof(vertDPI));
	uint16_t horzDPI;
	is.read(reinterpret_cast<char*>(&horzDPI), sizeof(horzDPI));
	char palette[48];
	is.read(palette, 48);
	is.get();
	uint8_t colorPlanes = is.get();
	uint16_t bytesPerPlaneLine;
	is.read(reinterpret_cast<char*>(&bytesPerPlaneLine), sizeof(bytesPerPlaneLine));
	uint16_t paletteInfo;
	is.read(reinterpret_cast<char*>(&paletteInfo), sizeof(paletteInfo));
	uint16_t horScrSize;
	is.read(reinterpret_cast<char*>(&horScrSize), sizeof(horScrSize));
	uint16_t verScrSize;
	is.read(reinterpret_cast<char*>(&verScrSize), sizeof(verScrSize));
	char padding[54];
	is.read(padding, 54);

	uint16_t width = windowXMax - windowXMin + 1;
	uint16_t height = windowYMax - windowYMin + 1;
	img.resize(height, width);
	uint32_t totalBytes = colorPlanes * bytesPerPlaneLine;
	int red_index = 0;
	int green_index = 0;
	int blue_index = 0;
	size_t red = 0;
	size_t green = 1;
	size_t blue = 2;
	for (int r = 0; r < img.rows(); r++) {
		uint32_t outputBytes = 0;
		uint16_t pixels = 0;
		bool re = true;
		bool gr = false;
		bool bl = false;
		bool first_change = true;
		while (outputBytes != totalBytes) {
			uint8_t x = is.get();
			uint8_t count = (x >> 6) == 3 ? x & 63 : 1;
			uint8_t data = (x >> 6) == 3 ? is.get() : x;
			for (uint8_t i = 0; i < count; i++) {
				if (pixels >= bytesPerPlaneLine) {
					pixels = 0;
					if (first_change) {
						if (re) {
							re = false;
							gr = true;
						}
						else if (gr) {
							gr = false;
							bl = true;
						}
						else if (bl) {
							bl = false;
							re = true;
						}
					}
					first_change = true;
				}
				if (pixels < bytesPerPlaneLine) {
					if (re) {
						if (pixels < width) {
							assert(red_index < img.size());
							img.data()[red_index++][red] = data;
						}
						else {
							if (first_change) {
								re = false;
								gr = true;
								first_change = false;
							}
						}
					}
					else if (gr) {
							if (pixels < width) {
								assert(green_index < img.size());
								img.data()[green_index++][green] = data;
							}
							else {
								if (first_change) {
									gr = false;
									bl = true;
									first_change = false;
								}
							}
					}
					else if (bl) {
							if (pixels < width) {
								assert(blue_index < img.size());
								img.data()[blue_index++][blue] = data;
							}
							else {
								if (first_change) {
									bl = false;
									re = true;
									first_change = false;
								}
							}
					}
					pixels++;
				}
				
				outputBytes++;
			}
		}
	}

	return true;
}