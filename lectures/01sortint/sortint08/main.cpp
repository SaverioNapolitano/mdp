#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <crtdbg.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <ranges>
#include <fstream> // Streams: generic version of files (they can be streams of whatever)
#include <iostream>
#include <sstream> // string stream 
#include <string>
#include <iterator>
#include <format> // from C++20 
#include <print> // from C++23
#include <set> // sorted container which will take the data and sort it keeping one copy for each element, implemented as a binary tree (inside uses red-black tree, aka auto-balancing tree to avoid degenerating cases)

/*
void print(std::ostream& os, const std::vector<int>& v)
{

    for (const auto& x : v) { // hard when you have to traverse multiple containers together, when you can use range-based for (clearer, less error-prone
        //fprintf(f, "%d\n", x);
        os << x << "\n";
    }

}
*/

// now everything is done from the constructor, we don't need it
std::vector<int> read(std::istream& is)
{
    using std::vector;
    /* we are passing a reference, no chance we pass a null object and thus no need for check
    if (f == nullptr) {
        return vector<int>();
    }
    */


    vector<int> v;
    int num;
    /*
    while (fscanf(f, "%d", &num) == 1) {
        v.push_back(num);
    }
    */

    while (is >> num) { // extractor operator which returns the stream itself, tries to read and if it succeds enters the while loop
        v.push_back(num);
    }
    // when we exit stream is in a failed state, we need to clear the state before reading again from it
    return v;
}



int main(int argc, char* argv[])
{
    using std::vector;
    // placement new (read this in a *good* book) 

    if (argc != 3) {
        // Problem: we could get wrong the type of the variable that we will write later as parameter
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        // C++ version 
        // Redefinition of operator shift left to insert something into an output stream (direct substitution for printf 
        // printf family of functions used to convert some binary data into text, in C++ when we want to convert a variable into a text representation we use the inserter operator << on a stream 
        // we can chain multiple operators together
        // we execute operators from left to right (most operators are left associative)
        // << returns the stream reference (not const) -> we return the same stream we received as first parameter 
        //std::cerr << "Usage: " << argv[0] << " <input file> <output file>\n"; 
        // cerr: error stream, cout: output stream 
        // formatting with inserter requires manipulators (ugly) 
        std::cerr << std::format("Usage: {} <input file> <output file>\n", argv[0]); //from C++20
        std::println("Usage: {} <input file> <output file>", argv[0]); //from C++23
        std::println(std::cerr, "Usage: {} <input file> <output file>", argv[0]);
        return 1;
    }
    //FILE* input = fopen(argv[1], "r");
    // std::ios::binary : don't convert new lines (ios: input output stream) -> ALWAYS WRITE IT AND THEN COMMENT IT OUT TO EXPLICITLY SAY WE ARE NOT READING IN BINARY MODE
    std::ifstream input(argv[1]/*, std::ios::binary*/); // ifstream: input file stream, if we read in text mode windows reads two bytes and gives us one byte for \n, in binary mode we read two bytes and get two bytes
    /*
    if (!input) {
        perror("Error opening input file");
        return 1;
    }
    */

    if (!input) { // !input.good() or input.fail() are equivalent 
        std::cerr << "Error opening input file\n";
        return 1;
    }
    /*
    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }
    */

    std::ofstream  output(argv[2]/*, std::ios::binary*/);
    if (!output) { // !input.good() or input.fail() are equivalent 
        std::cerr << "Error opening output file\n";
        return 1;
    }

    //vector<int> numbers; 

    
    std::istream_iterator<int> start(input);
    std::istream_iterator<int> stop; // default constructor 
    // Slightly more effective than inserting
    //vector<int> numbers(start, stop); // All containers in the std library have constructors with parameters (first_iterator, last_iterator)

    //vector<int> numbers(std::istream_iterator<int>(input), std::istream_iterator<int>()); // it doesn't work: The Most vexing parse problem (we define functions with keywords in many languages, in C/C++ we use parentheses with eventually some parameters with their type)
    // Usual rules for parsing prevent the compiler from understanding it is an object (parentheses can be confused with the definition of a function)
    // we can solve it changing the syntax: uniform initialization (no way to confuse {} with a function
    //vector<int> numbers{ std::istream_iterator<int>(input), std::istream_iterator<int>() };

    // set is sorted using a predicate that can be provided as a parameter to the constructor
    //std::set<int> numbers{ std::istream_iterator<int>(input), std::istream_iterator<int>() };

    // to keep multiple (all) copies of the same elements/objects
    std::multiset<int> numbers{ std::istream_iterator<int>(input), std::istream_iterator<int>() };

    //numbers[3] doesn't work in a set, we need to use iterators

    // Documentation too hard to understand, only way to know these tricks it's to check a good book (for instance, effective c++), read a lot of code from other people or ask chatgpt
    // every time we write to back_inserter we are pushing back to the vector 
    //std::copy(start, stop, std::back_inserter(numbers)); // substitutes the print we implemented 

    /* Compact version
    std::copy(
        std::istream_iterator<int>(input),
        std::istream_iterator<int>(),
        std::back_inserter(numbers)
    );
    */



    //numbers = read(input); // we can't copy streams (multiple copies of the same buffer would be inconsistent) -> use reference

    // Idea: we don't have generic functions that work on void pointers (we don't know how much data we have and how large a single element is)
    // We can "templatize" also functions 
    //qsort(numbers.data(), numbers.size(), sizeof(int), compare);

    // does not return anything, parameters RanIt stands for Random Access Iterator (we can use iterator and go to every possible position by just jumping) -> hard to have random access iterators (efficient mostly for vectors and arrays)
    // second version has an additional type parameter Pr (predicate) -> predicate allows us to choose how to sort 
    //std::sort(begin(numbers), end(numbers));
    // Having first and last iterators allows us to sort any kind of container based on ranges (more flexible than sorting all the container the same way)

    // it works also without std:: -> ADL: Argument Dependant Lookup (if we know that something exists we can use ADL to select it)
    //sort(begin(numbers), end(numbers)); // template function that understands it's part of std thanks to namespace of parameters -> it takes the function belonging to the same namespace
    // standard binary function is used, less operator that returns if the first number is less than the second one (it's not a comparator)

    /*
    print(output, numbers);
    print(std::cout, numbers); // cout is just ostream (output stream), different type from ofstream (which is a specialization of ostream)
    // same holds for istream and ifstream

    std::stringstream ss;
    print(ss, numbers);
    */

    /*
    for (const auto& x : numbers) { // hard when you have to traverse multiple containers together, when you can use range-based for (clearer, less error-prone
        //fprintf(f, "%d\n", x);
        output << x << "\n";
    }
    */

    // Algorithms allows us to copy, takes three iterators
    //int arr[10];
    // default implementation is as slow as a for loop, but before the compiler checks if it can copy the bits, if true it uses memmove (written in assembly to get maximum performance)
    // copy is using assignment operator, with streams we want to use inserter operator
    //copy(begin(numbers), end(numbers), std::begin(arr)); // begin(<int *>) returns an int* to the beginning of the array
    copy(begin(numbers), end(numbers), std::ostream_iterator<int>(output, "\n")); // we are copying all elements in the ostream_iterator, it will put every element into the output stream and add a \n after
    //copy(begin(numbers), end(numbers), std::ostream_iterator<int>(std::cout, ", ")); // we are copying all elements in the ostream_iterator, it will put every element into the output stream and add a \n after

    return 0;
}
