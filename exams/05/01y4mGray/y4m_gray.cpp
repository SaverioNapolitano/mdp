#include "mat.h"
#include <fstream>
#include <string>
bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
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
			mat<uint8_t> m(height, width);
			is.read(m.rawdata(), m.rawsize());
			frames.push_back(m);
			mat<uint8_t> dummy(height, width / 2);
			is.read(dummy.rawdata(), dummy.rawsize());
		}
		else {
			break;
		}
		tok = "    ";
	}
	return true;
}