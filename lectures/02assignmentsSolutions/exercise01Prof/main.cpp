#include <fstream>
#include <array> // wrapper around statically allocated stack array
#include <iomanip> // input output manipulator 

int main(int argc, char** argv) {

	using namespace std;

	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream is(argv[1], ios::binary);

	if (!is) {
		return EXIT_FAILURE;
	}

	// fastest solution, not flexible
	array<size_t, 256> count{}; // initialize array with 0s
	/*
	while (true) {
		int c = is.get();
		if (c == EOF) { // We failed to read one byte (that's why we don't read a single byte)
			break;
		}
		count[c]++;
	}
	*/
	// Compact version
	char c;
	while (is.get(c)) { // we are reading one byte into a char and returning the stream
		uint8_t u = c; // we can use implicit casting
		//++count[static_cast<uint8_t>(c)]; // we need to cast int to unsigned to avoid indexing array with negative numbers and convert a signed integer into an unsigned one
		++count[u]; 
	}
	
	

	ofstream os(argv[2]/*, std::ios::binary*/);

	if (!os) {
		return EXIT_FAILURE;
	}

	// FF useful to check if you botched the data type 
	// Spaces useful to check if your code eats white spaces -> use extractor only if you want to read text, they are designed to skip whitespaces and convert data to text
	// we can solve it with is.unsetf(ios::skipws) (don't do it, if you want to read one byte use a function that reads byte)
	// 0 useful because is the terminator

	for (size_t i = 0; i < 256; i++) {
		if (count[i] > 0) {
			// setw sets how many characters large it should be
			// setfill fills the characters in case there are not enough
			os << hex << setw(2) << setfill('0') << i << dec << '\t' << count[i] << '\n';
		}
	}

	// When we cast we change the representation of the data 
	// casting should be done very rarely
	// we can search for static_cast when we need to find where we did it
	double d = 3.14;
	// static cast changes one type into the other but doesn't work for pointers
	double e = static_cast<int>(d); // (int)d; C version

	 
	unsigned char arr[4] = {0xaa, 0xbb, 0xcc, 0xdd};
	// reinterpret cast changes the meaning of the pointer -> avoid them as much as possible, but it's needed to manage memory directly and to work with byte-level data representation
	// it doesn't work if arr is const unsigned char
	const int* p = reinterpret_cast<int*>(arr); //int *p = (int*)arr; // (int*) it's not making any transformation, it's just reinterpreting the bytes in memory (aka reinterpreting the meaning) -> we can't do a static cast
	//const int i = *p; // instead of 0xaabbccdd we see 0xddccbbaa because we are working with little endian -> when working with stuff in memory or on disk be careful
	
	//*p = 0; doesn't work if p is const// it writes 4 bytes equal to 0 in the array
	int* m = const_cast<int*>(p);

	return EXIT_SUCCESS;


}