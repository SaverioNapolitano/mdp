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

void print(FILE* f, const std::vector<int>& v)
{
    //for (size_t i = 0; i < v.size(); i++) { //to use when traversing multiple containers together 
    //    fprintf(f, "%d\n", v[i]);
    //}

    //std::vector<int>::const_iterator start = v.begin();
    //std::vector<int>::const_iterator stop = v.end();
    //std::vector<int>::const_iterator it = start;
    //while (it != stop) {
    //    int x = *it;
    //    fprintf(f, "%d\n", x);
    //    ++it;
    //}

    //auto start = v.begin();
    //auto stop = v.end();
    //auto it = start;
    //while (it != stop) {
    //    const auto& x = *it;
    //    fprintf(f, "%d\n", x);
    //    ++it;
    //}

    //for (auto it = begin(v), stop = end(v); it != stop; ++it) {
    //    const auto& x = *it;
    //    fprintf(f, "%d\n", x);
    //}

    for (const auto& x : v) { // hard when you have to traverse multiple containers together, when you can use range-based for (clearer, less error-prone
        fprintf(f, "%d\n", x);
    }

}

std::vector<int> read(FILE* f)
{
    using std::vector;

    if (f == nullptr) {
        return vector<int>();
    }

    vector<int> v;
    int num;
    while (fscanf(f, "%d", &num) == 1) {
        v.push_back(num);
    }
    return v;
}

int global_id = 0;
struct widget {
    int id_;
    int x_;
    widget() {
        id_ = global_id++;
        x_ = 5;
    }
    widget(int value) {
        id_ = global_id++;
        x_ = value;
    }
    widget(const widget& other) {
        id_ = global_id++;
        x_ = other.x_;
    }
    widget& operator=(const widget& other) {
        x_ = other.x_;
        return *this;
    }
    ~widget() {
    }

    bool operator<(const widget& rhs) const { // USE CONST METHODS AND CONST REFERENCES AS MUCH AS POSSIBLE 
        return x_ < rhs.x_;
    }

};

// Comparison function for qsort
/*
bool compare(int a, int b) // for int the reference is not needed (anything that is not a basic type uses const reference)
{
    return a > b;
}
*/

// functor -> we want comparator to behave as a function (e.g. we have to define the operator ()) 
// function -> somethng that can be called with ()
struct comparator {
    // your state information 
    int origin_;

    auto operator()(int a, int b) const
    {
        return abs((long long)a - origin_) < abs((long long)b - origin_);
    }

};

// Instead of specifying the return type of the function we may make it depends on the parameters (useful when we don't know what it is the return type until we know the type of the parameters)
// auto <funcname>(<parameters>) -> <type>, auto is required because the first thing that you get in a definition is a type
// compiler can get return type from the return statement 
// we can drop -> <type> if we have only one return 
// we could use -> int if we wanted 0 or 1
auto compare(int a, int b) // for int the reference is not needed (anything that is not a basic type uses const reference)
{
    return a > b;
}



int main(int argc, char* argv[])
{
    using std::vector;
    // placement new (read this in a *good* book) 

    /*
    
    // Sorting widgets 
    std::vector<widget> v;

    v.push_back(widget(5));
    v.push_back(widget(9));
    v.push_back(widget(-3));
    // There is no less than operator between widget -> we need to specify a predicate or we can specify a less than operator 
    // Sometimes we don't have any reasonable default less than
    sort(begin(v), end(v)); // Errors in templates are horrible to read 
    // default less tries to use the default lower than operator -> to sort something we need to provide the lower than operator for that thing
    */

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }
    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file");
        return 1;
    }
    FILE* output = fopen(argv[2], "w");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }

    vector<int> numbers;
    numbers = read(input);
    fclose(input);

    vector<double> v;
    v.push_back(3.1);
    v.push_back(4.56);
    v.push_back(0.1);

    // Idea: we don't have generic functions that work on void pointers (we don't know how much data we have and how large a single element is)
    // We can "templatize" also functions 
    //qsort(numbers.data(), numbers.size(), sizeof(int), compare);

    // does not return anything, parameters RanIt stands for Random Access Iterator (we can use iterator and go to every possible position by just jumping) -> hard to have random access iterators (efficient mostly for vectors and arrays)
    // second version has an additional type parameter Pr (predicate) -> predicate allows us to choose how to sort 
    //std::sort(begin(numbers), end(numbers));
    // Having first and last iterators allows us to sort any kind of container based on ranges (more flexible than sorting all the container the same way)

    // it works also without std:: -> ADL: Argument Dependant Lookup (if we know that something exists we can use ADL to select it)
    sort(begin(numbers), end(numbers)); // template function that understands it's part of std thanks to namespace of parameters -> it takes the function belonging to the same namespace
    // standard binary function is used, less operator that returns if the first number is less than the second one (it's not a comparator)


    // if we want to sort the numbers with a different rule we need to specify the predicate (a function for sorting) 
    // function must behave as a lower than operator
    sort(begin(numbers), end(numbers), compare);

    // with qsort we can't have a compare functions with context (additional information from outside) -> we want something with a state and a comparison function 
    // We need to build a function object (functor)
    //comparator comp; // useful if we need to do something with comp before the function 
    //sort(begin(numbers), end(numbers), comp); // comparator is a type, not an object on which we can call a method
    // sort will use quicksort unless there are few (< 32, Microsoft implementation) elements, otherwise it uses insertion sort (more efficient) 

    // we would like to write the comparison stuff in-place (lambda function) 
    // we substitute the auto keyword and the function name with []
    //sort(begin(numbers), end(numbers), [](int a, int b) {return a > b; });
    // to provide some context to comparison function (e.g. want to sort numbers based on their distance from a point (origin)
    long long origin = 2'000'000'001;
    // origin is not in the scope of the function (it won't work)
    //sort(begin(numbers), end(numbers), [](int a, int b) {return abs(a - origin) < abs(b - origin); });
    //comparator comp; // it has only a int, no initialization -> constructor doesn't need to do anything 
    //comp.origin_ = origin;
    //comparator comp{ origin }; // initializing the struct which has just plain old data -> uniform initialization (it will call the constructor if it's there but it also works with plain old data)
    // if we have the constructor no automatic conversion is done, the types must be exactly the same 

    //Inside the square brackets we can list the things we want to import (better with a reference, separated by a comma if more than one) into our context (context of a lambda = closure) -> closure = function object we are implicitly creating
    sort(begin(numbers), end(numbers), [&origin](int a, int b) {return abs(a - origin) < abs(b - origin); });
    // if we use [&] we take every variable mentioned in the lambda and put them in the []

    //using begin and end is flexible because you can sort parts, but most of the time we want to sort it all 

    print(output, numbers);

    fclose(output);

    return 0;
}
