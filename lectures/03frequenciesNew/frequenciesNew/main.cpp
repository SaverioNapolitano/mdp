#include <fstream>
#include <array> // wrapper around statically allocated stack array
#include <iomanip> // input output manipulator 
#include <map>
#include<unordered_map> // hashtables (hashmaps)
int main(int argc, char** argv) {

	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream is(argv[1], ios::binary);

	if (!is) {
		return EXIT_FAILURE;
	}

	// fixed types for key and content
	map<uint8_t, size_t> count; // maps are sorted on keys

	// Compact version
	char c;
	while (is.get(c)) { // we are reading one byte into a char and returning the stream
		uint8_t u = c; // we can use implicit casting
		// if we try to access a key that is not there the key is created with a default value
		// if we want to check if something is in the map we can't use [], we must use find method
		++count[u];
	}

	ofstream os(argv[2]/*, std::ios::binary*/);

	if (!os) {
		return EXIT_FAILURE;
	}

	for (const auto& x : count) { // We are iterating on pairs (items)
		// when you use a char or short in an expression they get automatically converted to int (we could use +x.first)
		// when you put a char into a stream it will be printed the char, we need to cast it to int
		os << hex << setw(2) << setfill('0') << int(x.first) << dec << '\t' << x.second << '\n';
	}

	return EXIT_SUCCESS;
}