#include <print>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <exception>

struct pos_exception : public std::exception {
	std::string message_;
	size_t pos_;
	pos_exception(std::string message, size_t pos) : message_(message), pos_(pos) {
		message_ += " @ pos: " + std::to_string(pos);
	}

	const char* what() const override {
		return message_.c_str(); // if we just cast we miss the string terminator
	}
};
struct elem { // elem wastes lots of memory since it allocates it also for things it won't use (we could use union to solve it but it's hard to handle since no constructor is called and we have to decide which one to call every time)
	// union requires "placement new"
	// std::variant (struct with a type and a list of possible types placed inside a union)
	char type_; // s,i,l,d
	std::string s_;
	int64_t i_ = 0;
	// elem e; error because the size is unknown (recursive definition of element is not possible since it could have other elements inside) 
	std::vector<elem> l_; // it works because it's a pointer (we know the size) and vector manages elements of unknown size
	std::map<std::string, elem> d_;


	// to put elements inside containers we often need the ability to default construct the object 
	// for vectors and maps we need a default construcctor 
	elem(){} // needed to allow maps and vectors to work
	elem(std::istream& is) {
		char next = is.peek();
		if (!is) {
			// it's a constructor, special function that doesn't return anything 
			// the only way to report errors it to return an object in an invalid state or to throw an exception 
			// we can throw a variable (object, int, string, ...) 
			//throw std::runtime_error("Unexpected read error");
			throw pos_exception("Unexpected read error", is.tellg());
		}
		if (next == 'i') {
			is >> type_ >> i_ >> next; // we are extracting the char we peeked, then the integer, then the 'e' that signals the end of the encoding
		}
		else if (next == 'l') {
			type_ = is.get(); // we save the character we peeked ('l')
			while (is.peek() != 'e') {
				l_.emplace_back(is); // push_back which takes as parameters the arguments of the constructor
			}
			is.get(); // we remove the final 'e'
		}
		else if (next == 'd') {
			type_ = is.get(); // we save the character we peeked ('d')
			while (is.peek() != 'e') {
				// read string 
				auto s = elem(is).s_; // we use the constructor to read an element (which we know will be a string) and then copy it
				// read element 
				d_[s] = elem(is); // we have to ensure we first read the string and then the element 

			}
			is.get(); // we remove the final 'e'
		}
		else {
			type_ = 's';
			// strings in C++ are sequences of bytes (can contain whatever, also 0 that in C string has a special meaning)
			size_t len;
			is >> len; // we use >> because the len is encoded as text and we want to convert it to a numeric value 
			if (is.get() != ':') {
				//throw std::runtime_error("Expected a :");
				throw pos_exception("Expected a :", is.tellg());
			}
			s_.resize(len);
			is.read(s_.data(), len);
		}
	}
	void print(size_t tabs = 0, const std::string& name = "") const {
		if (name == "pieces") {
			std::print("\"");
			for (size_t i = 0; i < size(s_); i +=20) { // size is a template method, ssize (signed size) if you need it to avoid casting manually
				std::print("{}", std::string(tabs + 1, '\t'));
				for (size_t j = 0; j < 20; j++) {
					std::print("{:02x}", s_[i + j]);
				}
				std::print("\n");
			}
		}
		else if (type_ == 's') {
			std::print("\"");
			for (const auto& c : s_) {
				if (c < 32 || c > 126) {
					std::print(".");
				}
				else {
					std::print("{}", c);
				}
			}
			std::print("\"\n");
		}
		else if (type_ == 'i') {
			std::print("{}\n", i_);
		}
		else if (type_ == 'l') {
			std::print("[\n");
			for (const auto& e : l_) {
				std::print("{}", std::string(tabs + 1, '\t'));
				e.print(tabs + 1);
			}
			std::print("{}", std::string(tabs + 1, '\t'));
			std::print("]\n");
		}
		else if (type_ == 'd') {
			std::print("{{\n"); // we need to escape '{' since it's a special character
			for (const auto& [name, e] : d_) { // we can leverage the tuple extension since iterating over maps gives us pairs
				std::print("{}\"{}\" => ", std::string(tabs + 1, '\t'), name);
				e.print(tabs + 1, name);
			}
			std::print("}}\n"); // we need to escape '}' since it's a special character
		}
	}
};



int main(int argc, char **argv) {
	if (argc != 2) {
		return EXIT_FAILURE;
	}

	std::ifstream is(argv[1], std::ios::binary);
	if (!is) {
		return EXIT_FAILURE;
	}
	// try block acts as a container, variable defined inside it are not available outside
	try {
		elem metainfo(is);
		metainfo.print();
	}
	catch(const std::exception& e) { // We must say what we want to catch, ... to specify we want to capture anything (but then we don't have any information), if we want to catch we should do it by const reference
		// here we are using polymorphism (we'll get the what of our exception and not the base class' one)
		std::print("{}", e.what());
	}

	return EXIT_SUCCESS;
}