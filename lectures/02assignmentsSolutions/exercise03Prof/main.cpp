#include <fstream>
#include <array> // wrapper around statically allocated stack array
#include <iomanip> // input output manipulator 
#include <vector>
#include <ranges>

// Wrapper to avoid writing reinterpret cast every time
template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

int main(int argc, char** argv) {

	using namespace std;


	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream is(argv[1]/*, ios::binary */);

	if (!is) {
		return EXIT_FAILURE;
	}

	vector<int> v;
	int32_t val;
	while (raw_read(is, val)) {
		v.push_back(val);
		
	}
	//vector<int> v{ istream_iterator<int>(is), istream_iterator<int>() };



	ofstream os(argv[2]/*, std::ios::binary*/);

	if (!os) {
		return EXIT_FAILURE;
	}

	ranges::copy(v, ostream_iterator<int>(os, "\n"));
	
	return EXIT_SUCCESS;


}