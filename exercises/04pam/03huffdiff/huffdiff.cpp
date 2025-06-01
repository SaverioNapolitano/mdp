#include <fstream>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <string>
#include <tuple>
#include <print>
#include <ostream>
#include <bit>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitwriter {
	std::ostream& os_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	std::ostream& write_bit(uint32_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		n_++;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
		return os_;
	}

public:
	bitwriter(std::ostream& os) :os_(os) {}
	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			write_bit(bit);
		}
		return os_;
	}
	~bitwriter() {
		flush();
	}
	std::ostream& operator()(uint32_t u, size_t n) {
		while (n-- > 0) {
			write_bit(u >> n);
		}
		return os_;
	}
};

class bitreader {
	std::istream& is_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	uint32_t read_bit() {
		if (n_ == 0) {
			buffer_ = is_.get();
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) :is_(is) {}
	~bitreader() {}
	std::istream& operator()(uint32_t& u, size_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}
};

template<typename T>
struct frequency {
	std::unordered_map<T, uint32_t> counter_;

	void operator()(const T& val) {
		counter_[val]++;
	}
};

template<typename T>
struct huffman {
	struct node {
		static inline int id = 0;
		int id_;
		T sym_;
		uint32_t freq_, code_, len_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(T sym, uint32_t freq) :sym_(std::move(sym)), freq_(freq), id_(id++) {}
		node(node* left, node* right) :freq_(left->freq_ + right->freq_), left_(left), right_(right), id_(id++) {}

		bool operator<(const node& other) const {
			if (other.freq_ == freq_) {
				return id_ > other.id_;
			}
			return freq_ > other.freq_;
		}
	};

	std::unordered_map<T, node*> map_;
	std::vector<std::unique_ptr<node>> mem_;
	std::vector<std::tuple<T, uint32_t, uint32_t>> codes_; // sym, code, len

	void make_codes() {
		uint32_t c = 0;
		uint32_t l = 0;
		for (auto& [sym, code, len] : codes_) {
			while (l < len) {
				c = c << 1;
				l++;
			}
			code = c;
			map_[sym]->code_ = code;
			c++;
		}
	}

	void find_len(node* n, uint32_t len) {
		if (n->left_ == nullptr) {
			n->len_ = len;
			map_[n->sym_] = n;
			codes_.push_back({ n->sym_, 0, len });
		}
		else {
			find_len(n->left_, len + 1);
			find_len(n->right_, len + 1);
		}
	}

	template<typename It>
	huffman(It first, It last) {
		auto f = std::for_each(first, last, frequency<T>{});

		auto cmp = [](node* a, node* b) {return *a < *b; };

		std::priority_queue<node*, std::vector<node*>, decltype(cmp)> nodes(cmp);

		for (const auto& [sym, freq] : f.counter_) {
			mem_.push_back(std::make_unique<node>(sym, freq));
			nodes.push(mem_.back().get());
		}

		while (nodes.size() > 1) {
			auto n1 = nodes.top();
			nodes.pop();
			auto n2 = nodes.top();
			nodes.pop();
			mem_.push_back(std::make_unique<node>(n1, n2));
			nodes.push(mem_.back().get());
		}

		auto root = nodes.top();
		nodes.pop();

		find_len(root, 0);



		/*
		std::sort(begin(codes_), end(codes_), [](const std::tuple<T, uint32_t, uint32_t>& a, const std::tuple<T, uint32_t, uint32_t>& b)
			{if (std::get<2>(a) == std::get<2>(b)) { return std::get<0>(a) < std::get<0>(b); } return std::get<2>(a) < std::get<2>(b); });

		*/

		std::sort(begin(codes_), end(codes_), [](const std::tuple<T, uint32_t, uint32_t>& a, const std::tuple<T, uint32_t, uint32_t>& b)
			{ return std::get<2>(a) < std::get<2>(b); });

		make_codes();
	}

};

template<typename T>
struct image {
	size_t rows_;
	size_t cols_;
	std::vector<T> data_;

	image(size_t rows, size_t cols) :rows_(rows), cols_(cols), data_(rows* cols) {}

	T& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}

	T& operator[](size_t i) {
		return data_[i];
	}

	char* rawdata() {
		return reinterpret_cast<char*>(data_.data());
	}

	size_t rawsize() {
		return rows_ * cols_ * sizeof(T);
	}

	auto begin() { return data_.begin(); }
	auto end() { return data_.end(); }
};

image<uint8_t> read_image(std::istream& is) {
	std::string token(2, ' ');
	size_t width = 0, height = 0;
	while ((is >> token) && (token != "ENDHDR")) {
		if (token == "WIDTH") {
			is >> width;
		}
		else if (token == "HEIGHT") {
			is >> height;
		}
		else if (token == "DEPTH") {
			int depth;
			is >> depth;
		}
		else if (token == "MAXVAL") {
			int maxval;
			is >> maxval;
		}
		else if (token == "TUPLTYPE") {
			std::string tupltype;
			is >> tupltype;
		}
		else {
			std::string dummy;
			std::getline(is, dummy);
		}
	}
	is.get();
	image<uint8_t> im(height, width);
	is.read(im.rawdata(), im.rawsize());
	return im;
}

void compress(const std::string& infile, const std::string& outfile) {
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		exit(EXIT_FAILURE);
	}

	image<uint8_t> im = read_image(is);

	image<uint16_t> diff(im.rows_, im.cols_);

	for (size_t r = 0; r < im.rows_; r++) {
		for (size_t c = 0; c < im.cols_; c++) {
			if (r == 0 && c == 0) {
				diff(r, c) = im(r, c);
			}
			if (c == 0 && r > 0) {
				diff(r, c) = im(r, c) - im(r - 1, c);
			}
			if (c > 0) {
				diff(r, c) = im(r, c) - im(r, c - 1);
			}
		}
	}

	huffman<uint16_t> h(diff.begin(), diff.end());

	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		exit(EXIT_FAILURE);
	}
	os << "HUFFDIFF";
	raw_write(os, im.cols_, 4);
	raw_write(os, im.rows_, 4);
	uint16_t table_size = h.map_.size();
	bitwriter bw(os);
	bw(table_size, 9);
	for (const auto& [sym, code, len] : h.codes_) {
		bw(sym, 9);
		bw(len, 5);
	}

	for (const auto& x : diff) {
		auto n = h.map_[x];
		bw(n->code_, n->len_);
	}

}

void write_image(image<uint8_t>& im, std::ostream& os) {
	std::print(os, "P7\nWIDTH {}\nHEIGHT {}\nDEPTH 1\nMAXVAL 255\nTUPLTYPE GRAYSCALE\nENDHDR\n", im.cols_, im.rows_);
	os.write(im.rawdata(), im.rawsize());
}

void decompress(const std::string& infile, const std::string& outfile) {
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		exit(EXIT_FAILURE);
	}
	std::string header(8, ' ');
	raw_read(is, header[0], 8);
	size_t width = 0, height = 0;
	raw_read(is, width, 4);
	raw_read(is, height, 4);
	image<uint16_t> diff(height, width);
	bitreader br(is);
	uint32_t table_len;
	br(table_len, 9);

	using table_entry = std::tuple<uint16_t, uint32_t, uint32_t>; // sym, code, len
	std::vector<table_entry> table;

	for (size_t i = 0; i < table_len; i++) {
		uint32_t sym, code, len;
		br(sym, 9);
		br(len, 5);
		table.emplace_back(sym, 0, len);
	}

	uint32_t c = 0;
	uint32_t l = 0;
	for (auto& [sym, code, len] : table) {
		while (l < len) {
			c = c << 1;
			l++;
		}
		code = c;
		c++;
	}

	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		exit(EXIT_FAILURE);
	}
	for (size_t r = 0; r < height; r++) {
		for (size_t c = 0; c < width; c++) {
			size_t pos;
			uint32_t len = 0, code = 0;
			bool found = false;
			for (pos = 0; pos < table_len; pos++) {
				while (len < std::get<2>(table[pos])) {
					uint32_t bit;
					br(bit, 1);
					code = (code << 1) | bit;
					len++;
				}
				if (code == std::get<1>(table[pos])) {
					found = true;
					break;
				}
			}
			if (!found) {
				exit(EXIT_FAILURE);
			}
			diff(r, c) = std::get<0>(table[pos]);
		}
	}

	image<uint8_t> im(height, width);
	for (size_t r = 0; r < height; r++) {
		for (size_t c = 0; c < width; c++) {
			if (r == 0 && c == 0) {
				im(r, c) = diff(r, c);
			}
			if (c == 0 && r > 0) {
				im(r, c) = diff(r, c) + im(r - 1, c);
			}
			if (c > 0) {
				im(r, c) = diff(r, c) + im(r, c - 1);
			}
		}
	}

	write_image(im, os);
}

int main(int argc, char** argv) {
	using namespace std::string_literals;
	if (argc != 4) {
		return EXIT_FAILURE;
	}
	if (argv[1] == "c"s) {
		compress(argv[2], argv[3]);
	}
	else if (argv[1] == "d"s) {
		decompress(argv[2], argv[3]);
	}
	else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}