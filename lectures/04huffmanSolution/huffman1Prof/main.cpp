#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <print>
#include <queue>
#include <memory>

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
		T sym_;
		uint32_t freq_;
		uint32_t code_, len_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		// since we can do something with sym in the body, the compiler won't move the object but copy it
		// we can use a type cast using std::move() to tell the compiler it can move it
		// We pass just T because we may (or may not) copy it (the compiler will decide if copy or move it)
		// If parameters can be moved because we know they're not going to be used we have to specify it to the compiler
		node(T sym, uint32_t freq) : sym_(std::move(sym)), freq_(freq) {}
		node(node* left, node* right) :freq_(left->freq_ + right->freq_), left_(left), right_(right) {}
	};

	struct nodeptr {
		static inline int id = 0; // not to use if we want to share class among multiple files 
		int id_;
		node* p_;
		nodeptr(node* p):p_(p), id_(id++) {}
		operator node* () { // we are defining the operator cast
			return p_;
		}
		//node* operator->() { return p_; }
		//const node* operator->() const { return p_; }

		// deducing this [property/feature]
		template<typename Self> 
		auto&& operator->(this Self&& self) { return self.p_; }

		bool operator<(const nodeptr& other) const {
			if (p_->freq_ == other.p_->freq_) {
				return id_ > other.id_;
			}
			return p_->freq_ > other.p_->freq_;
		}

	};

	void make_codes(node* n, uint32_t code, uint32_t len) {
		if (n->left_ == nullptr) { // we are in a leaf
			n->code_ = code;
			n->len_ = len;
			map[n->sym_] = n; // we can access the map thanks to the implicit this parameter
		}
		else {
			make_codes(n->left_, code << 1, len + 1);
			make_codes(n->right_, (code << 1) | 1, len + 1);
		}
	}

	std::unordered_map<uint8_t, node*> map;
	std::vector<std::unique_ptr<node>> mem_; // the destructor of huffman calls the destructor of the vector 
	// it's a vector of object, thus the destructor of vector calls the destructor of the objects inside the vector

	template<typename It>
	huffman(It first, It last) {
		auto f = std::for_each(first, last, frequency<uint8_t>{});

		// priority queue gives the highest element, to get the reverse order we have to reverse the predicate
		// every predicate must behave as a lower than operator (can't be possible to swap elements)
		// We need to assign an id to each node when we create it and use it in the comparison -> if frequency is the same we must return first the one with the smallest id
		std::priority_queue<nodeptr> nodes;
		// priority queue has no idea of where we are inserting -> if many elements have same probability ?
		// priority queue is not sorted, but we are guaranteed that the first element has the lowest value

		//std::vector<node<uint8_t>*> nodes;
		for (const auto& [sym, freq] : f.counter_) {
			//auto n = new node(sym, freq);
			//auto n = std::make_unique<node>(sym, freq);
			//mem_.push_back(n); // it doesn't work, we are trying to copy a unique pointer
			mem_.push_back(std::make_unique<node>(sym, freq)); 
			//nodes.push_back(n);
			//nodes.push(n.get());
			nodes.push(mem_.back().get()); // structure that uses the pointer but never deallocates it 
			// the owner of the memory is the vector mem_
		}

		//std::sort(begin(nodes), end(nodes), pred);

		while (nodes.size() > 1) {
			//auto n1 = nodes.back();
			//nodes.pop_back();
			//auto n2 = nodes.back();
			//nodes.pop_back();

			auto n1 = nodes.top();
			nodes.pop();
			auto n2 = nodes.top();
			nodes.pop();
			//auto n = new node(n1, n2);
			mem_.push_back(std::make_unique<node>(n1, n2));
			//nodes.push(n);
			nodes.push(mem_.back().get());

			//auto it = std::lower_bound(begin(nodes), end(nodes), n, pred);

			//nodes.insert(it, n);
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
/*
template<typename T>
struct node {
	T sym_;
	uint32_t freq_;
	uint32_t code_, len_;
	node* left_ = nullptr;
	node* right_ = nullptr;

	// since we can do something with sym in the body, the compiler won't move the object but copy it
	// we can use a type cast using std::move() to tell the compiler it can move it
	// We pass just T because we may (or may not) copy it (the compiler will decide if copy or move it)
	// If parameters can be moved because we know they're not going to be used we have to specify it to the compiler
	node(T sym, uint32_t freq) : sym_(std::move(sym)),freq_(freq) {}
	node(node*left, node*right):freq_(left->freq_ + right->freq_), left_(left), right_(right) {}
};
*/

/*
template<typename T>
void make_codes(node<T>* n, uint32_t code, uint32_t len, std::unordered_map<T, node<T>*>& map) {
	if (n->left_ == nullptr) { // we are in a leaf
		n->code_ = code;
		n->len_ = len;
		map[n->sym_] = n;
	}
	else {
		make_codes(n->left_, code << 1, len + 1, map);
		make_codes(n->right_, (code << 1) | 1, len + 1, map);
	}
}
*/


int main(int argc, char** argv) {
	using namespace std::string_literals;
	/*
	{
		//int* p = new int;
		// automatic deleting pointer -> no memory leaks
		//std::unique_ptr<int> p; // unique_ptr knows it's not pointing to anything 

		//auto s = sizeof(p); // unique_ptr doesn't add anything to the pointer (we pay nothing in memory, just a little overhead at run-time since the destructor is called even when the pointer is null)


		//p.reset(new int);  // set the content of p to some place in memory
		//auto p = std::make_unique<int>(7);
		//*p = 8;
		//int* q = p;
		//int* q = p.get(); // do this every time we need to pass the pointer to a function that is gonna use the pointer but not deleting it
		//delete p;
		//delete q; // we are deleting the same memory twice
		//*q = 9; // UB = Undefined Behaviour (we are using a piece of memory that has already been released
		// Problem: we didn't explicitly say who is the owner of the memory and who is the user of the memory
		// We have also to establish 
		//std::unique_ptr<int> q;
		//q = p; // can't copy unique_ptr (otherwise we would have two owners for the same memory)
		//q = std::move(p); // pass the ownership (p will be empty after this) -> only the delete for q will happen
		// if we want two owners for the same memory (e.g., multi-threading application,
		// data structures with multiple pointers to the same location) -> when we don't know the order of destruction
		// shared ownership -> single (shared) counter that needs to be updated
		auto p = std::make_shared<int>(7); // it has a significant run time cost because of atomic counters
		// atomic counters -> locks require accessing the operating system 
		auto s = sizeof(p); // size of the pointer is doubled (one reference to the data + one reference to the counter)
		*p = 8;
		std::shared_ptr<int>q;
		q = p;
		// shared pointer easier to use
	}
	*/
 	if (argc != 4 or (argv[1] != "c"s and argv[1] != "d"s)) {
		return EXIT_FAILURE;
	}
	if (argv[1] == "c"s) {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		// fastest way (not very flexible) to read the file
		is.seekg(0, std::ios::end); // to jump at the end of the file (we start from position 0 and move of std::ios::end positions)
		auto filesize = is.tellg(); // tells the position in the file, since we moved to the end it's the file size
		is.seekg(0); // we need to go back to the beginning of the file
		// filesize is of type pos_type but we can use it as an integer
		std::vector<uint8_t> v(filesize);
		raw_read(is, v[0], filesize);

		/*
		frequency<uint8_t> f;
		for (const auto& x : v) {
			f(x);
		}
		*/

		/*
		auto f = std::for_each(begin(v), end(v), frequency<uint8_t>{});

		using nodeptr = node<uint8_t>*;
		auto pred = [](const nodeptr a, const nodeptr b) {return a->freq_ > b->freq_; };
		// priority queue gives the highest element, to get the reverse order we have to reverse the predicate
		// every predicate must behave as a lower than operator (can't be possible to swap elements)
		// We need to assign an id to each node when we create it and use it in the comparison -> if frequency is the same we must return first the one with the smallest id
		std::priority_queue<nodeptr, std::vector<nodeptr>, decltype(pred)> nodes(pred);
		// priority queue has no idea of where we are inserting -> if many elements have same probability ?
		// 

		//std::vector<node<uint8_t>*> nodes;
		for (const auto& [sym, freq] : f.counter_) {
			auto n = new node(sym, freq);
			//nodes.push_back(n);
			nodes.push(n);
		}

		//std::sort(begin(nodes), end(nodes), pred);

		while (nodes.size() > 1) {
			//auto n1 = nodes.back();
			//nodes.pop_back();
			//auto n2 = nodes.back();
			//nodes.pop_back();

			auto n1 = nodes.top();
			nodes.pop();
			auto n2 = nodes.top();
			nodes.pop();
			auto n = new node(n1, n2);
			nodes.push(n);

			//auto it = std::lower_bound(begin(nodes), end(nodes), n, pred);

			//nodes.insert(it, n);
		}
		// Applies a function to every element of the sequence passed to it
		// it copies things passed to a local variable (we need to create a new frequency object)
		// the return type will be the same as the one we passed as parameter 
		
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}
		//auto root = nodes.back();
		//nodes.pop_back();
		auto root = nodes.top();
		nodes.pop();

		std::unordered_map<uint8_t, node<uint8_t>*> map;

		make_codes(root, 0, 0, map);
		*/

		huffman<uint8_t> h(begin(v), end(v));

		std::vector<huffman<uint8_t>::node*> nodes;
		for (const auto& [sym, n] : h) {
			nodes.push_back(n);
		}
		std::sort(begin(nodes), end(nodes), [](const huffman<uint8_t>::node* a, const huffman<uint8_t>::node* b){
			return a->len_ < b->len_;
			});
		/*
		for (auto& n : nodes) { // :<padding><width><format>
			if (isprint(n->sym_)) {
				std::println("{:c} {:0{}b}", n->sym_, n->code_, n->len_); // :<pad with 0><
			}
			else {
				//std::println("0x{:x02} {:0{}b}", n->sym_, n->code_, n->len_);
				std::println("{:02x} {:0{}b}", n->sym_, n->code_, n->len_);
			}

		}
		*/
		
		/*
		for (const auto& [sym, n] : h.map) { // :<padding><width><format>
			if (isprint(sym)) {
				std::println("{:c} {:0{}b}", sym, n->code_, n->len_); // :<pad with 0><
			}
			else {
				std::println("0x{:x02} {:0{}b}", sym, n->code_, n->len_);
			}
			
		}
		*/
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}

		os << "HUFFMAN1";
		uint8_t table_size = static_cast<uint8_t>(h.size());
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
	else {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
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
		//typedef std::tuple<uint8_t, uint32_t, uint32_t> table_entry;
		using table_entry = std::tuple<uint8_t, uint32_t, uint32_t>; // sym, code, len
		std::vector<table_entry> table;
		bitreader br(is);
		for (size_t i = 0; i < table_len; ++i) {
			//table_entry entry;
			uint32_t sym, code, len;
			br(sym, 8);
			br(len, 5);
			br(code, len);
			//table.push_back(table_entry(sym, code, len));
			table.emplace_back(sym, code, len); // directly creates the object inside the vector instead of creating a temporary object and then copying it
			// it passes the parameters to the constructor of the type defined in the vector
			/*
			get<0>(entry) = sym;
			get<1>(entry) = len;
			get<2>(entry) = code;
			*/
		}
		uint32_t n;
		br(n, 32);
		sort(begin(table), end(table), [](const table_entry& a, const table_entry& b) {return get<2>(a) < get<2>(b); });

		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}

		for (uint32_t i = 0; i < n; ++i) {
			size_t pos;
			uint32_t code = 0;
			uint32_t len = 0;
			bool found = false;
			for (pos = 0; pos < table_len; ++pos) {
				while (get<2>(table[pos]) > len) {
					uint32_t bit;
					br(bit, 1);
					code = (code << 1) | bit;
					++len;
				}
				if (code == get<1>(table[pos])) {
					found = true;
					break;
				}
			}
			if (!found) {
				return EXIT_FAILURE;
			}
			os.put(get<0>(table[pos]));
		}
		
	}

	return EXIT_SUCCESS;
}

// MOST OF THE TIME WE DON'T NEED TO CALL NEW
// Weak pointers -> reference to an object which does not delete but prevent deletion (we are not owning the object but we don't allow it to be deleted)