#include <fstream>
#include <array> // wrapper around statically allocated stack array
#include <iomanip> // input output manipulator 
#include <vector>

// Wrapper to avoid writing reinterpret cast every time
template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

// AT THE EXAM YOU MUST BE ABLE TO WRITE THE CLASS YOURSELF
// In a class everything is private, in a struct everything is public -> if you need everything public just write a struct
class bitwriter {
//public: //It's equivalent to a struct
	std::ostream& os_;
	uint8_t buffer_ = 0; // Initialized for every construcctor
	size_t n_ = 0; // Initialized for every constructor 

	void writebit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1); // we are taking the least significant bit of bit, removing the least significant bit from buffer and then inserting it 
		//buffer_ = buffer_ * 2 + (bit & 1) // it's equivalent since mathematically the shift is defined as a multiplication by 2 and the bit & 1 will always be 0 or 1 without carry and the least significant bit in buffer will always be 0 after the shift
		++n_;
		if (n_ == 8) {
			os_.put(buffer_);
			// raw_write(os_, buffer_);
			n_ = 0;
		}
	}

public:
	// Things get initialized in the same order they are defined in the class
	bitwriter(std::ostream& os) : os_(os) {
		//os_ = os; // Can't assign to a reference (reference can only be initialized) -> constructore can have initializer list
		
	}
	~bitwriter() {
		flush();
	}

	// Write the n least significant bits of u from MSB to LSB (MSB = Most Significant Bit, LSB = Least Significant Bit)
	std::ostream& write(uint32_t u, size_t n) {
		/*
		* TRY NOT TO HAVE WARNINGS IN YOUR CODE
		for(int i= static_cast<int>(n); i >= 0; i--){
			writebit(u >> i);
		}
		*/
		while (n --> 0) {
			writebit(u >> n);
		}
		return os_;
	}
	// We rename write function to write less code
	std::ostream& operator()(uint32_t u, size_t n) {
		/*
		* TRY NOT TO HAVE WARNINGS IN YOUR CODE
		for(int i= static_cast<int>(n); i >= 0; i--){
			writebit(u >> i);
		}
		*/
		while (n-- > 0) {
			writebit(u >> n);
		}
		return os_;
	}

	// pad (default is with 0)
	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) { // if the buffer is empty (n_ = 0) no padding is required
			writebit(bit);
		}
		return os_;
	}
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

	vector<int> v{ istream_iterator<int>(is), istream_iterator<int>() }; // elegant but slow



	ofstream os(argv[2]/*, std::ios::binary*/);

	if (!os) {
		return EXIT_FAILURE;
	}

	bitwriter bw(os);
	for (const auto& x : v) {
		bw(x, 11);
	}

	// Object destruction is in the reverse order of creation (we are sure the bitwriter will be destroyed after the output stream)
	// The destructor flushes automatically for us
	
	return EXIT_SUCCESS;
}