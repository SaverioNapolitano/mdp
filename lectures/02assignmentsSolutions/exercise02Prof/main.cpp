#include <fstream>
#include <array> // wrapper around statically allocated stack array
#include <iomanip> // input output manipulator 
#include <vector>

// Wrapper to avoid writing reinterpret cast every time
template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

int main(int argc, char** argv) {

	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream is(argv[1]/*, ios::binary */ );

	if (!is) {
		return EXIT_FAILURE;
	}

	vector<int> v{ istream_iterator<int>(is), istream_iterator<int>() };

	

	ofstream os(argv[2]/*, std::ios::binary*/);

	if (!os) {
		return EXIT_FAILURE;
	}
	/* Manual version(machine independent), if we have a big endian machine it will still write in little endian
	for (const auto& x : v) {
		os.put((x >> 0) & 0xff); // we are taking the lowest eight bits of the number and we put it into output 	
		os.put((x >> 8) & 0xff);
		os.put((x >> 16) & 0xff);
		os.put((x >> 24) & 0xff);
	}
	*/

	/* Compact version
	for (const auto& x : v) {
		//const int* p = &x; 
		//const char* c = reinterpret_cast<const char*>(p);
		// Compact version
		os.write(reinterpret_cast<const char*>(&x), 4); // it takes bytes in memory and it writes them from memory to file (one byte at a time, doesn't interpret anything) -> we are writing 4 bytes starting from p
	}
	*/

	// More compact version 
	os.write(reinterpret_cast<const char*>(v.data()), v.size() * sizeof(int)); // v.data() is the address of the first element, starting from there we write all the vector

	// Wrapper versions
	/*
	for (const int& x : v) {
		raw_write(os, x);
	}
	*/
	//raw_write(os, v); // WRONG, DON'T DO IT (we are taking the address of the vector class
	raw_write(os, v[0], v.size() * sizeof(int)); 

	/*
	int i = 0xaa66bb77;
	// we are writing in little endian, thus the first byte we need is the least significant one
	// binary end bits by bits with 
	// we are writing in little endian 
	os.put((i >> 0) & 0xff); // we are taking the lowest eight bits of the number and we put it into output 	
	os.put((i >> 8) & 0xff);
	os.put((i >> 16) & 0xff);
	os.put((i >> 24) & 0xff);
	*/

	return EXIT_SUCCESS;


}