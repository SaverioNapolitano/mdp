#include <fstream>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <string>
#include <tuple>

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
	bitwriter(std::ostream& os): os_(os) {}
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
	bitreader(std::istream& is): is_(is) {}
	~bitreader() {}
	std::istream& operator()(uint32_t& u, size_t n) {
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
		uint32_t freq_, len_, code_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(T sym, uint32_t freq): sym_(std::move(sym)), freq_(freq), id_(id++) {}
		node(node*left,node*right):freq_(left->freq_+right->freq_),left_(left),right_(right),id_(id++){}
		bool operator<(const node& other) const {
			if (freq_ == other.freq_) {
				return id_ > other.id_;
			}
			return freq_ > other.freq_;
		}
	};

	std::unordered_map<T, node*> map_;
	std::vector<std::unique_ptr<node>> mem_;
	std::vector<std::tuple<T, uint32_t, uint32_t>> codes_; // sym, len, code

	void find_len(node* n, uint32_t len) {
		if (n->left_ == nullptr) {
			n->len_ = len;
			map_[n->sym_] = n;
		}
		else {
			find_len(n->left_, len + 1);
			find_len(n->right_, len + 1);
		}
	}

	void make_codes() {
		for (const auto& [sym, n] : map_) {
			codes_.push_back({ sym, n->len_, 0 });
		}
		std::sort(begin(codes_), end(codes_), [](const std::tuple<T, uint32_t, uint32_t>& a, 
			const std::tuple<T, uint32_t, uint32_t>& b) {if (std::get<1>(a) == std::get<1>(b)) 
		{ return std::get<0>(a) < std::get<0>(b); } return std::get<1>(a) < std::get<1>(b); });

		uint32_t c = 0;
		uint32_t l = 0;
		for (auto& [sym, len, code] : codes_) {
			while (l < len) {
				c <<= 1;
				l++;
			}
			code = c;
			map_[sym]->code_ = code;
			c++;
		}
	}

	template<typename It>
	huffman(It first, It last) {
		auto f = std::for_each(first, last, frequency<T>{});

		auto cmp = [](const node* a, const node* b) {return *a < *b; };
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

		make_codes();

	}
	
};

void compress(const std::string& infile, const std::string& outfile) {
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		exit(EXIT_FAILURE);
	}
	is.seekg(0, std::ios::end);
	auto filesize = is.tellg();
	is.seekg(0);
	std::vector<uint8_t> v(filesize);
	raw_read(is, v[0], filesize);
	huffman<uint8_t> h(begin(v), end(v));

	using table_entry = std::pair<uint8_t, uint32_t>; // sym, len
	std::vector<table_entry> table;
	
	for (const auto& [sym, len, code] : h.codes_) {
		table.push_back({ sym, len});
	}
	std::sort(begin(table), end(table), [](const table_entry& a, const table_entry& b) {if (a.second == b.second) { return a.first < b.first; } return a.second < b.second; });
	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		exit(EXIT_FAILURE);
	}
	os << "HUFFMAN2";
	uint8_t table_size = h.codes_.size();
	os.put(table_size);
	bitwriter bw(os);
	for (const auto& [sym, len] : table) {
		bw(sym, 8);
		bw(len, 5);
	}
	uint32_t num = v.size();
	bw(num, 32);
	for (const auto& x : v) {
		auto n = h.map_[x];
		bw(n->code_, n->len_);
	}
	
}

void decompress(const std::string& infile, const std::string& outfile) {
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		exit(EXIT_FAILURE);
	}
	std::string header(8, ' ');
	raw_read(is, header[0], 8);
	size_t table_size = is.get();
	if (table_size == 0) {
		table_size = 256;
	}
	using table_entry = std::pair<uint8_t, uint32_t>; // sym, len
	std::vector<table_entry> table;
	bitreader br(is);
	for (size_t i = 0; i < table_size; i++) {
		uint32_t sym = 0, len = 0;
		br(sym, 8);
		br(len, 5);
		table.emplace_back(sym, len);
	}

	std::vector<std::tuple<uint8_t, uint32_t, uint32_t>> codes; // sym, code, len
	
	for (const auto& [sym, len] : table) {
		codes.push_back({ sym, 0, len });
	}

	std::sort(begin(codes), end(codes), [](const auto& a, const auto& b)
		{if (std::get<2>(a) == std::get<2>(b)) { return std::get<0>(a) < std::get<0>(b); } return std::get<2>(a) < std::get<2>(b); });

	uint32_t c = 0;
	uint32_t l = 0;
	for (auto& [sym, code, len] : codes) {
		while (l < len) {
			c <<= 1;
			l++;
		}
		code = c;
		c++;
	}

	uint32_t n = 0;
	br(n, 32);
	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		exit(EXIT_FAILURE);
	}
	for (size_t i = 0; i < n; i++) {
		size_t pos;
		uint32_t code = 0, len = 0;
		bool found = false;
		for (pos = 0; pos < table_size; pos++) {
			while (len < std::get<2>(codes[pos])) {
				uint32_t bit = 0;
				br(bit, 1);
				code = (code << 1) | bit;
				len++;
			}
			if (code == std::get<1>(codes[pos])) {
				found = true;
				break;
			}
		}
		if (!found) {
			exit(EXIT_FAILURE);
		}
		os.put(std::get<0>(codes[pos]));
	}
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