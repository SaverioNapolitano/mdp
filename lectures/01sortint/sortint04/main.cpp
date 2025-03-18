#define _CRT_SECURE_NO_WARNINGS
//#include <stdio.h>
//#include <stdlib.h>
//#include <assert.h>
// The right way to include C headers in C++ programs
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <crtdbg.h> //it slightly changes the definition of some memory allocation functions such that in debug mode it keeps track of all the memory allocation that has been done

int sortNumbers(const void* a, const void* b) {
	const int* x = (const int*)a;
	const int* y = (const int*)b;

	return (*x > *y) - (*x < *y);
}

// By defining the functions inside the definition of the struct the compiler knows they will operate on it
// namespace: part of the name of the function given by the compiler (can be omitted or explicitly written)
// C++ will automatically put for every method a pointer to the object (no need to write it), we can use this implicitly
// this is a vector * 
// We can use just the name of the structure even without typedef 
// If you need a destructor, copy constructor or assignment operator (one of them) then you need all of them (the fourth is the default constructor)
struct vector { // it's at least 24 bytes since we are compiling for x64 architecture (compiler might put some spaces in between or after the end to align in memory, never puts space before)
	size_t size_, capacity_; //Google convention: _ after variable name Microsoft: m_ before variable name (m stands for member)
	int* data_;
	// for every pointer we must define who is in charge of managing that resource (e.g., who is the owner)
	// constructor: function that initializes the object
	// default constructor
	vector() { // we need to tell the compiler this method is the constructor by naming the method as the class
		size_ = 0; // if there is no ambiguity we can drop this->
		capacity_ = 10;
		data_ = (int*)malloc(capacity_ * sizeof(int)); // Type system is stricter, we can't use type void freely, we need to convert it explicitly
	}
	// copy is another form of construct 
	// copy constructor 
	vector(const vector& other) { // ALWAYS WRITE CONST THEN REMOVE IT IF SOMETHING DOES NOT WORK
		size_ = other.size_;
		capacity_ = other.capacity_;
		data_ = (int*)malloc(capacity_ * sizeof(int));
		for (size_t i = 0; i < size_; i++) {
			data_[i] = other.data_[i];
		}

	}
	// copy constructor 
	vector& operator=(const vector& other) { // We changed from void copy(...) to vector& operator=(...), vector& is a vector reference
		if (capacity_ < other.size_) { // to avoid allocating new memory when unnecessary (if this is true this can't be a self assignment)
			free(data_); //if we don't do this we get a memory leak because we allocate new memory without freeing the one allocated previously
			capacity_ = other.capacity_;
			data_ = (int*)malloc(capacity_ * sizeof(int));
		}
		/*
		PREVIOUS CHECK FOR SELF ASSIGNMENT
		if (this == &other) { // if the addresses are the same we are doing a self-assignment
			return *this; // if we don't do this we free the data and then try to copy them
		}*/
		size_ = other.size_;
		// inefficient in case of self-assignment, but it's unlikely
		for (size_t i = 0; i < size_; i++) {
			data_[i] = other.data_[i];
		}
		// we can't return this since it's a vector * (pointer)
		return *this; // we dereference this in order to return the object pointed to by it since we need it to initialize the reference

	}

	~vector() { // this is the destructor
		free(data_); //if we allow the copy, we are telling that when we copy also the data will be copied
	}

	void push_back(int num) {
		if (size_ == capacity_) {
			capacity_ *= 2;
			data_ = (int*)realloc(data_, capacity_ * sizeof(int));;
		}
		data_[size_] = num;
		size_++;
	}

	size_t size() const { // we are communicating to the user that size() is a const method (can be called on a read-only object)
		return size_; // if we try this->size++ it will throw an error 
	}

	int at(size_t index) const { // it tells that the object is not modifiable
		assert(index < size_); // If you compile in debug mode it does the check, it disappears in release mode
		return data_[index];
	}
};
// The idea is to have a pointer that automatically dereferences the object (a reference to something in memory, e.g. a hidden pointer) -> & syntax
// we are not copying anything, we are just making an alias with &v
// we must initialize the reference (can't have NULL references) and we can't change the initial initialization (reference is frozen)
void print(FILE* f, vector const& v) { // if we use vector v numbers and v are two different variables (different addresses in memory) but point to the same location (data)
	for (size_t i = 0; i < v.size(); i++) { // if we use vector v we pass a struct to a function and in C++ (like C) it is shallow copied (just the surface is copied, e.g. the bytes), if we have pointers to something that needs to be duplicated it won't work
		fprintf(f, "%d\n", v.at(i)); // we can solve it by passing a pointer (*v) instead of v (we are not copying anything but the pointer), but is ugly because we have to use the -> syntax and the & in the main
	}
} //if we use vector v when we get out the vector v is freed and thus the original data is deallocated 
// if we use the reference we will eliminate just the reference 
// WE WANT TO AVOID COPYING AS MUCH AS POSSIBLE

// methods of an object are not inside the object, they are just functions logically related to the object 

int main(int argc, char** argv) {
	//size_t x = sizeof(vector);
	{
		if (argc != 3) {
			return 1;
		}
		FILE* filein = fopen(argv[1], "r");
		if (filein == NULL) {
			return 1;
		}
		FILE* fileout = fopen(argv[2], "w");
		if (fileout == NULL) {
			fclose(filein);
			return 1;
		}

		vector numbers; // definition of a variable, the compiler will call the constructor for us automatically

		int num;
		while (fscanf(filein, "%d", &num) == 1) {
			numbers.push_back(num);
		}

		fclose(filein);
		//vector original(numbers) is equivalent to vector original = numbers
		//vector original = numbers; // definition and initialization
		// If we want to customise the behaviour of something we need to customise the behaviour of the operator
		//vector original; definition
		//original = number; assignment -> original and number have the same address, because we are using the = operator, we are not initializing anything (we are copying bytes, shallow copy)
		//CORRECT VERSION 
		// vector original; //Here we created the object and allocated some memory
		//original.copy(numbers);
		// After name change 
		// original.operator=(numbers);
		// Or
		// original = numbers; //doing = in C++ means calling the operator= method of the object, it takes as first parameter the object itself and as second parameter a const reference to the object we want to assign
		vector original, another;
		another = original = numbers; //does not work because the function returns void, we need to return the object we are modifying (e.g. the one to which we assign something) 
		// In C++ the result of an assignment is not the value being assigned but the object to which you have assigned (memory address of the variable you have assigned)
		numbers = numbers; //self assignment doesn't work because we are first freeing the data, then allocating data and copying from other data -> we need to catch it in the operator= definition
		qsort(numbers.data_, numbers.size(), sizeof(int), sortNumbers);

		print(fileout, numbers);

		fclose(fileout);
		// Automatic resource management
	} // All the vectors are out of scope thus the destructor will be called when we get here
	_CrtDumpMemoryLeaks(); //It returns the allocated memory which haven't been deallocated up to this point, it dumps them in the Output Window
	return 0; //when we exit the scope of the variable the compiler will call the destructor for us, double free problem if we used a shallow copy

}