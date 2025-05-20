#include "mat.h"
#include "types.h"
#include <string>
#include <fstream>

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string stream_header;
	is >> stream_header;
	if (stream_header != "YUV4MPEG2") {
		return false;
	}
	char token = ' ';
	int width = 0, height = 0, frame_rate = 0, form_factor = 0;
	char interlace = 'p';
	std::string chroma_subsampling("420jpeg");
	std::string app_field;
	while (token != '\n') {
		token = is.get();
		if (token == 'W') {
			is >> width;
		}
		if (token == 'H') {
			is >> height;
		}
		if (token == 'C') {
			is >> chroma_subsampling;
		}
		if (token == 'I') {
			interlace = is.get();
		}
		if (token == 'F') {
			is >> frame_rate;
		}
		if (token == 'A') {
			is >> form_factor;
		}
		if (token == 'X') {
			is >> app_field;
		}
	}
	if (interlace != 'p' || chroma_subsampling != "420jpeg") {
		return false;
	}
	if (width == 0 || height == 0) {
		return false;
	}
	std::string tok;
	while (is) {
		is >> tok;
		if (tok == "FRAME") {
			char c = ' ';
			while (c != '\n') {
				c = is.get();
			}
			mat<uint8_t> y(height, width);
			mat<uint8_t> cb(height, width);
			mat<uint8_t> cr(height, width);
			for (int r = 0; r < height; r++) {
				for (int c = 0; c < width; c++) {
					uint8_t b = is.get();
					if (b < 16) {
						b = 16;
					}
					if (b > 235) {
						b = 235;
					}
					y(r, c) = b;
				}
			}
			for (int r = 0; r < height; r+=2) {
				for (int c = 0; c < width; c+=2) {
					uint8_t b = is.get();
					if (b < 16) {
						b = 16;
					}
					if (b > 240) {
						b = 240;
					}
					cb(r, c) = b;
					cb(r, c + 1) = b;
					cb(r + 1, c) = b;
					cb(r + 1, c + 1) = b;
				}
			}
			for (int r = 0; r < height; r+=2) {
				for (int c = 0; c < width; c+=2) {
					uint8_t b = is.get();
					if (b < 16) {
						b = 16;
					}
					if (b > 240) {
						b = 240;
					}
					cr(r, c) = b;
					cr(r, c + 1) = b;
					cr(r + 1, c) = b;
					cr(r + 1, c + 1) = b;
				}
			}

			mat<vec3b> frame(height, width);
			for (int r = 0; r < height; r++) {
				for (int c = 0; c < width; c++) {
					double red = 1.164 * (y(r, c) - 16) + 1.596 * (cr(r, c) - 128);
					double green = 1.164 * (y(r, c) - 16) - 0.392 * (cb(r, c) - 128) - 0.813 * (cr(r, c) - 128);
					double blue = 1.164 * (y(r, c) - 16) + 2.017 * (cb(r, c) - 128);
					if (red < 0) {
						red = 0;
					}
					if (red > 255) {
						red = 255;
					}
					if (green < 0) {
						green = 0;
					}
					if (green > 255) {
						green = 255;
					}
					if (blue < 0) {
						blue = 0;
					}
					if (blue > 255) {
						blue = 255;
					}
					vec3b pixel{ static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue) };
					frame(r, c) = pixel;
				}
			}
			frames.push_back(frame);
		}
		else {
			break;
		}
		tok = "    ";
	}
	return true;
}