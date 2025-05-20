#include <crtdbg.h>
#include <fstream>

#include "shapes.h"

std::vector<std::unique_ptr<shape>>
load_shapes(const std::string& filename) 
{
	std::vector<std::unique_ptr<shape>> shapes;
	std::ifstream is(filename/*, std::ios::binary*/);
	if (is) {
		std::string token;
		int x, y, x1, y1, r;
		char c;
		while (is >> token) { // switch used only for some optimization, works only for integer types
			if (token == "rectangle") {
				is >> x >> y >> x1 >> y1 >> c;
				shapes.push_back(std::make_unique<rectangle>(x, y, x1, y1, c));
			}
			else if (token == "point") {
				is >> x >> y >> c;
				shapes.push_back(std::make_unique<point>(x, y, c));
			}
			else if (token == "line") {
				is >> x >> y >> x1 >> y1 >> c;
				shapes.push_back(std::make_unique<line>(x, y, x1, y1, c));
			}
			else if (token == "circle") {
				is >> x >> y >> r >> c;
				shapes.push_back(std::make_unique<circle>(x, y, r, c));
			}
			else {
				getline(is, token);
			}
		}
	}
	return shapes; // Thanks to return value optimization the object we created is already the one outside (no copy)
}

int main(void)
{
	using namespace std;
	{
		canvas c(80, 25);

		auto shapes = load_shapes("drawing1.txt");

		for (const auto& s : shapes) {
			s->draw(c);
		}

		c.out(stdout);
	}
	_CrtDumpMemoryLeaks();
}
