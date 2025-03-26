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

class bitreader {
	//public: //It's equivalent to a struct
	std::istream& is_;
	uint8_t buffer_ = 0; // Initialized for every construcctor
	size_t n_ = 0; // Initialized for every constructor 

	uint32_t readbit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1; // We push the bit we want to read in the last position and then do & 1 to keep only that
		// return buffer_ / (1 << n_) % 2; equivalent ( 1 << n_ is like 2^n_)
	}

public:
	// Things get initialized in the same order they are defined in the class
	bitreader(std::istream& is) : is_(is) {
	}

	// Read n bits of u from MSB to LSB (MSB = Most Significant Bit, LSB = Least Significant Bit)
	std::istream& operator()(uint32_t& u, size_t n) {
		u = 0;
		while (n --> 0) {
			u = (u << 1) | readbit();
		}
		return is_;
	}
	// Because of flushing/padding we could read more bytes than what we wrote, but the state of the stream takes care of that 
};

int main(int argc, char** argv) {

	using namespace std;


	if (argc != 3) {
		return EXIT_FAILURE;
	}

	ifstream is(argv[1]/*, ios::binary */);

	if (!is) {
		return EXIT_FAILURE;
	}

	bitreader br(is);
	vector<int> v;
	uint32_t val;
	// bitfields (valid both for C and C++)
	while (br(val, 11)) {
		int32_t realnumber = val;
		if (val > 1023) {
			realnumber = val - 2048;
		}
		v.push_back(realnumber);

	}
	//vector<int> v{ istream_iterator<int>(is), istream_iterator<int>() };



	ofstream os(argv[2]/*, std::ios::binary*/);

	if (!os) {
		return EXIT_FAILURE;
	}

	ranges::copy(v, ostream_iterator<int>(os, "\n")); // ultra slow in debug mode

	return EXIT_SUCCESS;


}