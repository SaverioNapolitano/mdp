#include <print>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <cmath>
//#include <string>
//#include <bit>
class bitwriter {
	std::ostream& os_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;

	std::ostream& writebit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		n_++;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) : os_(os) {}
	~bitwriter() {
		flush();
	}

	std::ostream& operator()(uint32_t u, size_t n) {
		while (n-- > 0) {
			writebit(u >> n);
		}
		return os_;
	}

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			writebit(bit);
		}
		return os_;
	}
};

class bitreader {
	std::istream& is_;
	uint8_t buffer_ = 0;
	size_t n_ = 0;

	uint32_t readbit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) : is_(is) {};

	std::istream& operator()(uint32_t& u, size_t n) {
		//u = 0;
		while (n-- > 0) {
			u = (u << 1) | readbit();
		}
		return is_;
	}

	/* Instead of checking the input stream we can check bitreader with !br
	operator bool() const {
		return bool(is_); // return is_.good();
	}
	*/


};


/*
uint32_t map(int32_t x){
	// ...
}
*/
/*
void compress(const std::string& infile, const std::string& outfile) {
	// use exit() to signal big errors from a void function
	// ...
}

void unmap(...){
	// ...
}

void decompress(...){
	// ...
}
*/



int main(int argc, char** argv) {
	if (argc != 4 || (argv[1][0] != 'c' && argv[1][0] != 'd')) {
		std::println(std::cerr, "Usage: {} [c|d] <filein> <fileout>", argv[0]);
		return 1;
	}
	// using namespace std::string_literals;
	if (argv[1][0] == 'c') { // argv[1] == "c"s to define a const string // DON'T DO argv[1][0] because it would work also with "cheese" and it shouldn't
		std::ifstream is(argv[2]/*, std::ios::binary*/);
		if (!is) {
			std::cerr << "Error opening input file\n";
			return 1;
		}
		std::vector<int> numbers{ std::istream_iterator<int>(is), std::istream_iterator<int>() };
		
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			std::cerr << "Error opening output file\n";
			return 1;
		}
		bitwriter bw(os);

		// map(x) = -2x if x < 0 else 2x + 1
		for (auto& x : numbers) {
			x = x < 0 ? x *= (-2) : 2 * x + 1;
			size_t trailingZeros = static_cast<size_t>(std::floor(std::log2(x))); // Alternative idea: the number of bits needed is (the position of the most significant 1) * 2 - 1 (we can find it by shifting to the right the number until we find a 0)
			// there is a function for that -> bit_width() defined in header bit
			bw(0, trailingZeros);
			bw(static_cast<uint32_t>(x), trailingZeros + 1); // bw(static_cast<uint32_t>(x), std::bit_width(static_cast<uint32_t>(x)) * 2 - 1);
		}
	}
	else {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			std::cerr << "Error opening input file\n";
			return 1;
		}
		bitreader br(is);
		std::ofstream os(argv[3]/*, std::ios::binary*/);
		if (!os) {
			std::cerr << "Error opening output file\n";
			return 1;
		}
		uint32_t val = 0;
		size_t trailingZeros = 0;
		while (br(val, 1)) {
			if (val == 0) {
				trailingZeros++;
			}
			else {
				br(val, trailingZeros);
				// map(realnum) = -0.5realnum if realnum % 2 == 0 else 0.5(realnum - 1)
				int realnum = static_cast<int>(val);
				realnum = realnum % 2 == 0 ? -0.5 * realnum : 0.5 * (realnum - 1);
				os << realnum << "\n";
				val = 0;
				trailingZeros = 0;
			}
		}
	}

	return 0;

}