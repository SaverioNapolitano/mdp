#include <string>
#include <fstream>
#include <map>
#include <cmath>
#include <cstdint>
#include <vector>

#define MAGIC_NUMBER "LZ78"

class bitwriter {
	std::ostream& os_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	std::ostream& writebit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		n_++;
		if (n_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
			n_ = 0;
		}
		return os_;
	}

public:

	bitwriter(std::ostream& os):os_(os){}
	~bitwriter(){
		flush();
	}
	std::ostream& operator()(uint32_t u, size_t n) {
		while (n -- > 0) {
			writebit(u >> n);
		}
		return os_;
	}
	void flush(uint32_t bit = 0) {
		while (n_ > 0) {
			writebit(bit);
		}
	}
};
bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {
	using namespace std;
	ifstream is(input_filename, ios::binary);
	if (!is) {
		return false;
	}
	if (maxbits < 1 or maxbits > 30) {
		return false;
	}
	ofstream os(output_filename, ios::binary);
	if (!os) {
		return false;
	}
	bitwriter bw(os);
	os.write(MAGIC_NUMBER, sizeof(MAGIC_NUMBER) - 1);
	bw(static_cast<uint32_t>(maxbits), 5);
	uint32_t index = 0;
	size_t bits = 0;
	uint32_t match = 0;
	map<string, uint32_t> dict{};
	dict[""] = index++;
	char c;
	string s{};
	
	while (is.get(c)) {
		s.push_back(c);
		bool matched = dict.contains(s);
		if (matched) {
			auto item = dict.find(s);
			match = item->second;
			matched = true;
		}
		if (dict.size() == 1) {
			bw(c, 8);
			bits++;
			dict[s] = index++;
			s.clear();
		}
		else {
			if (!matched) {
				dict[s] = index++;
				bw(match, bits);
				bw(c, 8);
				uint32_t p = pow(2, bits);
				if (dict.size() - 1 == p) {
					bits++;
				}
				if (bits > static_cast<size_t>(maxbits)) {
					dict.clear();
					index = 0;
					dict[""] = index++;
					bits = 0;
				}
				s.clear();
				match = 0;
			}
		}
		
	}
	if (dict[s] == match) {
		s.pop_back();
		if (dict.contains(s)) {
			auto item = dict.find(s);
			bw(item->second, bits);
			bw(c, 8);
		}
	}
	return true;

}