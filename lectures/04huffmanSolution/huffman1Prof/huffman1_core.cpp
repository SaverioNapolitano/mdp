#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <print>
#include <queue>
#include <memory>
#include <map>

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T))
{
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T))
{
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitwriter {
	std::ostream& os_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	std::ostream& writebit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		n_++;
		if (n_ == 8) {
			os_.put(buffer_);
			n_ = 0;
		}
		return os_;
	}

public:

	bitwriter(std::ofstream& os) :os_(os) {};
	~bitwriter() {
		flush();
	}
	std::ostream& flush(uint8_t bit = 0) {
		while (n_ > 0) {
			writebit(bit);
		}
		return os_;
	}
	std::ostream& operator()(uint32_t u, size_t n) {
		while (n-- > 0) {
			writebit(u >> n);
		}
		return os_;
	}
};

class bitreader {
	std::istream& is_;
	size_t n_ = 0;
	uint8_t buffer_ = 0;

	uint32_t readbit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
			n_ = 8;
		}
		n_--;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is) :is_(is) {};
	~bitreader() {};
	std::istream& operator()(uint32_t& u, size_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | readbit();
		}
		return is_;
	}
};

template<typename T>
struct frequency {
	// if we drop the requirement of having order on our map we can have some computational advantage 
	// unordered_map uses hashtables 
	std::unordered_map<T, uint32_t> counter_;

	void operator()(const T& val) {
		++counter_[val];
	}
};

template<typename T>
struct huffman {

	struct node {
		static inline int id = 0; // not to use if we want to share class among multiple files 
		int id_;
		T sym_;
		uint32_t freq_;
		uint32_t code_, len_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		// since we can do something with sym in the body, the compiler won't move the object but copy it
		// we can use a type cast using std::move() to tell the compiler it can move it
		// We pass just T because we may (or may not) copy it (the compiler will decide if copy or move it)
		// If parameters can be moved because we know they're not going to be used we have to specify it to the compiler
		node(T sym, uint32_t freq) : sym_(std::move(sym)), freq_(freq), id_(id++) {} // leaf node
		node(node* left, node* right) :freq_(left->freq_ + right->freq_), left_(left), right_(right), id_(id++) {} // intermediate node -> we don't care about sym

		bool operator<(const node& other) const {
			if (freq_ == other.freq_) {
				return id_ > other.id_;
			}
			return freq_ > other.freq_;
		}
	};

	std::unordered_map<T, node*> map;
	std::vector<std::unique_ptr<node>> mem_; // the destructor of huffman calls the destructor of the vector 
	// it's a vector of object, thus the destructor of vector calls the destructor of the objects inside the vector
	void make_codes(node* n, uint32_t code, uint32_t len) {
		if (n->left_ == nullptr) { // we are in a leaf
			n->code_ = code;
			n->len_ = len;
			map[n->sym_] = n; // we can access the map thanks to the implicit this parameter
		}
		else {
			make_codes(n->left_, code << 1, len + 1); // we add a 0 if we go left
			make_codes(n->right_, (code << 1) | 1, len + 1); // we add a 1 if we go right
		}
	}

	template<typename It>
	huffman(It first, It last) {
		// we are applying the operator() of the frequency struct to every item in the range [first, last)
		// we can do that because its return type is void and it takes a reference as parameter
		auto f = std::for_each(first, last, frequency<T>{}); 

		// priority queue gives the highest element, to get the reverse order we have to reverse the predicate
		// every predicate must behave as a lower than operator (can't be possible to swap elements)
		// We need to assign an id to each node when we create it and use it in the comparison -> if frequency is the same we must return first the one with the smallest id
		// priority queue is not sorted, but we are guaranteed that the first element has the lowest value
		auto cmp = [](const node* a, const node* b) {return *a < *b; }; // we need it because we are comparing node*, not node in the priority queue
		std::priority_queue<node*, std::vector<node*>, decltype(cmp)> nodes(cmp);

		// insert leaves
		for (const auto& [sym, freq] : f.counter_) {
			mem_.push_back(std::make_unique<node>(sym, freq));
			nodes.push(mem_.back().get()); // structure that uses the pointer but never deallocates it 
			// the owner of the memory is the vector mem_
		}

		// build tree
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

		make_codes(root, 0, 0);
	}

	auto begin() { return map.begin(); }
	auto end() { return map.end(); }
	auto size() { return map.size(); }
	auto operator[](const T& sym) { return map[sym]; }
};

void compress(const std::string& infile, const std::string& outfile) {
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		exit(EXIT_FAILURE);
	}
	// fastest way (not very flexible) to read the file
	is.seekg(0, std::ios::end); // to jump at the end of the file (we start from position 0 and move of std::ios::end positions)
	auto filesize = is.tellg(); // tells the position in the file, since we moved to the end it's the file size
	is.seekg(0); // we need to go back to the beginning of the file
	// filesize is of type pos_type but we can use it as an integer
	std::vector<uint8_t> v(filesize);
	raw_read(is, v[0], filesize);

	huffman<uint8_t> h(begin(v), end(v));

	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		exit(EXIT_FAILURE);
	}

	os << "HUFFMAN1";
	uint8_t table_size = h.size();
	os.put(table_size);
	bitwriter bw(os);
	for (const auto& [sym, n] : h) {
		bw(sym, 8);
		bw(n->len_, 5);
		bw(n->code_, n->len_);
	}
	bw(static_cast<uint32_t>(v.size()), 32);
	for (const auto& x : v) {
		auto n = h[x];
		bw(n->code_, n->len_);
	}
}

void decompress(const std::string& infile, const std::string& outfile) {
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		exit(EXIT_FAILURE);
	}

	std::string header(8, ' ');
	// WRONG -> is.read(reinterpret_cast<char *>(&header), 8);
	// string is an object, we are overwriting its fields and not its characters
	// WRONG -> raw_read(is, header, 8);
	// is.read(&header[0], 8); // OK
	is.read(header.data(), 8); // OK
	// raw_read(is, header[0], 8); OK

	size_t table_len = is.get();
	if (table_len == 0) {
		table_len = 256;
	}
	using table_entry = std::tuple<uint8_t, uint32_t, uint32_t>; // sym, code, len
	std::vector<table_entry> table;
	bitreader br(is);
	for (size_t i = 0; i < table_len; ++i) {
		uint32_t sym, code, len;
		br(sym, 8);
		br(len, 5);
		br(code, len);
		table.emplace_back(sym, code, len); // directly creates the object inside the vector instead of creating a temporary object and then copying it
		// it passes the parameters to the constructor of the type defined in the vector
	}
	uint32_t n;
	br(n, 32);
	sort(begin(table), end(table), [](const table_entry& a, const table_entry& b) {return get<2>(a) < get<2>(b); });

	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < n; ++i) {
		size_t pos;
		uint32_t code = 0;
		uint32_t len = 0;
		bool found = false;
		for (pos = 0; pos < table_len; ++pos) {
			while (get<2>(table[pos]) > len) { // while the actual len is lower than the len in the pos position we keep reading
				uint32_t bit;
				br(bit, 1);
				code = (code << 1) | bit;
				++len;
			}
			if (code == get<1>(table[pos])) { // once we reach the correct len we check if the code is the same (we can have more codes of the same len)
				found = true;
				break;
			}
		}
		if (!found) {
			exit(EXIT_FAILURE);
		}
		os.put(get<0>(table[pos]));
	}
}

int main(int argc, char** argv) {
	using namespace std::string_literals;

	if (argc != 4 or (argv[1] != "c"s and argv[1] != "d"s)) {
		return EXIT_FAILURE;
	}
	if (argv[1] == "c"s) {
		compress(argv[2], argv[3]);
	}
	else if(argv[1] == "d"s) {
		decompress(argv[2], argv[3]);
	}
	else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}