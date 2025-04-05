#include <string>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <set>
#include <ranges>

#define MagicNumber "HUFFMAN1"
#define symbols std::set<uint8_t>
#define node std::pair<symbols, uint32_t>
#define level std::vector<node>

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
};

class tree {
	level buildLeaves(std::ifstream& is) {
		char c;
		level l{};
		std::map<uint8_t, uint32_t> frequencies;
		while (is.get(c)) {
			uint8_t u = c;
			frequencies[u]++;
			/*
			symbols s{ u };
			size_t i = 0;
			if (!isNodeAlreadyPresent(l, s, &i)) {
				node n{ s, 1 };
				l.emplace_back(n);
			}
			else {
				l[i].second++;
			}
			*/
		}
		for (const auto& item : frequencies) {
			symbols s{ item.first };
			node n{ s, item.second };
			l.emplace_back(n);
		}
		std::ranges::sort(l, [](node a, node b) {return a.second < b.second; });
		return l;
	}
	/*
	bool isNodeAlreadyPresent(level l, symbols s, size_t* i) {
		for (size_t j = 0; j < l.size(); j++) {
			if (l[j].first == s) {
				*i = j;
				return true;
			}
		}
		return false;
	}
	*/
	

	void buildTree(level leaves) {
		level last_level = leaves;
		while (last_level.size() > 1) {
			symbols s1 = last_level[0].first;
			symbols s2 = last_level[1].first;
			s1.merge(symbols{ s2 });
			uint32_t f1 = last_level[0].second;
			uint32_t f2 = last_level[1].second;
			node n{ s1, f1 + f2 };
			last_level.erase(begin(last_level), begin(last_level) + 2);
			last_level.push_back(n);
			std::ranges::sort(last_level, [](node a, node b) {return a.second < b.second; });
			t_.push_back(last_level);
		}
	}

public:
	std::vector<level> t_{};
	tree() {}

	void buildFromFile(std::ifstream& is) {
		level leaves = buildLeaves(is);
		t_.push_back(leaves);
		buildTree(leaves);
	}

};

struct tableentry {
	uint8_t symbol = 0; 
	uint8_t len = 0;
	uint32_t code = 0;
};

std::vector<tableentry> getEntries(std::vector<level> t) {
	std::map<symbols, uint32_t> allCodes{};
	std::map<symbols, uint8_t> allLengths{};
	int shift = 0;
	for (size_t i = t.size() - 1; i > 0; i--) { // for each level
		level currentLevel = t[i];
		level nextLevel = t[i - 1];
		for (size_t j = 0; j < currentLevel.size(); j++) { // for each node in current level
			node currentNode = currentLevel[j];
			bool placed = false;
			for (size_t k = 0; k < nextLevel.size(); k++) { // for each node in next level
				node nextNode = nextLevel[k];
				if (std::ranges::includes(currentNode.first, nextNode.first) and currentNode.first != nextNode.first) {
					node otherNode = nextLevel[k + 1];

					allCodes[nextNode.first] = (allCodes[currentNode.first] << shift) | 1;
					allCodes[otherNode.first] = (allCodes[currentNode.first] << shift) | 0;
					shift = 1;

					allLengths[nextNode.first] = allLengths[currentNode.first] + 1;
					allLengths[otherNode.first] = allLengths[currentNode.first] + 1;
					placed = true;
					break;
				}
			}
			if (placed) {
				break;
			}
		}
	}
	std::vector<tableentry> huffmanTable{};
	for (const auto& item : allCodes) {
		if (item.first.size() == 1) {
			for (const auto& symbol : item.first) {
				tableentry e{};
				e.symbol = symbol;
				e.len = allLengths[item.first];
				e.code = item.second;
				huffmanTable.push_back(e);
			}
		}
	}
	return huffmanTable;
}

int main(int argc, char** argv) {
	using namespace std::string_literals;
	if (argc != 4 or (argv[1] != "c"s and argv[1] != "d"s)) {
		return EXIT_FAILURE;
	}
	if (argv[1] == "c"s) {
		/*
		if (argv[2] == "bibbia.txt"s) {
			return EXIT_FAILURE;
		}
		*/
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}

		bitwriter bw(os);

		tree tr{};
		tr.buildFromFile(is);
		std::vector<tableentry> huffmanTable = getEntries(tr.t_);

		uint8_t tableEntries = static_cast<uint8_t>(huffmanTable.size());

		level leaves = tr.t_[0];
		uint32_t numSymbols = 0;
		for (const auto& leaf : leaves) {
			numSymbols += leaf.second;
		}

		os.write(MagicNumber, sizeof(MagicNumber) - 1);
		os.put(tableEntries);

		for (const tableentry& triplets : huffmanTable) {
			bw(triplets.symbol, 8);
			bw(triplets.len, 5);
			bw(triplets.code, triplets.len);
		}

		bw(numSymbols, 32);

		is.clear();
		is.seekg(0);
		char c;
		while (is.get(c)) {
			uint8_t u = c;
			for (const tableentry& triplet : huffmanTable) {
				if (triplet.symbol == u) {
					bw(triplet.code, triplet.len);
					break;
				}
			}
		}
	}
	else {
		std::ifstream is(argv[2], std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}
		/*
		if (argv[3] == "bibbia_comp.bin"s) {
			return EXIT_FAILURE;
		}
		*/
		
		std::ofstream os(argv[3], std::ios::binary);
		if (!os) {
			return EXIT_FAILURE;
		}

		bitreader br(is);
		bitwriter bw(os);

		char magicnum[9];
		is.read(magicnum, 8);
		magicnum[8] = 0;
		if (std::string(magicnum) != MagicNumber) {
			return EXIT_FAILURE;
		}
		char c;
		is.get(c);
		uint8_t tableEntries = c;
		std::vector<tableentry> huffmanTable{};
		std::vector<std::vector<uint32_t>> table{};
		for (size_t i = 0; i < tableEntries; i++) {
			tableentry e;
			uint32_t symbol = 0;
			uint32_t len = 0;
			uint32_t code = 0;
			br(symbol, 8);
			br(len, 5);
			br(code, len);
			std::vector<uint32_t> entry;
			entry.push_back(symbol);
			entry.push_back(len);
			entry.push_back(code);
			table.push_back(entry);
			e.symbol = symbol;
			e.len = len;
			e.code = code;
			huffmanTable.push_back(e);
		}
		std::sort(begin(huffmanTable), end(huffmanTable), [](tableentry a, tableentry b) {return a.len < b.len; });

		uint32_t numSymbols;
		br(numSymbols, 32);
		uint32_t code = 0;
		uint8_t len = 0;
		for (size_t i = 0; i < numSymbols;) {
			br(code, 1);
			len++;
			tableentry e{};
			e.len = len;
			const auto p = std::equal_range(begin(huffmanTable), end(huffmanTable), e, [](tableentry a, tableentry b) {return a.len < b.len; });
			for (auto it = p.first; it != p.second; it++) {
				if (len == it->len && code == it->code) {
					bw(it->symbol, 8);
					code = 0;
					len = 0;
					i++;
					break;
				}
			}
		}
		
		
	}

	return EXIT_SUCCESS;
}