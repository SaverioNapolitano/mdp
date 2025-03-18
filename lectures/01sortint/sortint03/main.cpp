#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

struct vector { // it's at least 24 bytes since we are compiling for x64 architecture (compiler might put some spaces in between or after the end to align in memory, never puts space before)
	size_t size_, capacity_; //Google convention: _ after variable name Microsoft: m_ before variable name (m stands for member)
	int* data_;

	// constructor: function that initializes the object
	vector() { // we need to tell the compiler this method is the constructor by naming the method as the class
		size_ = 0; // if there is no ambiguity we can drop this->
		capacity_ = 10;
		data_ = (int*)malloc(capacity_ * sizeof(int)); // Type system is stricter, we can't use type void freely, we need to convert it explicitly
	}

	~vector() { // this is the destructor (~className is the standard way of defining it)
		free(data_);
	}

	void push_back(int num) {
		if (size_ == capacity_) {
			capacity_ *= 2;
			data_ = (int*)realloc(data_, capacity_ * sizeof(int));;
		}
		data_[size_] = num;
		size_++;
	}

	size_t size() const { // we are communicating to the user that size() is a const method (can be called on a read-only object
		return size_; // if we try this->size++ it will throw an error 
	}

	int at(size_t index) const { // it tells that the object is not modifiable
		assert(index < size_); // If you compile in debug mode it does the check, it disappears in release mode
		return data_[index];
	}
};



// methods of an object are not inside the object, they are just functions logically related to the object 

int main(int argc, char** argv) {
	//size_t x = sizeof(vector);

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

	qsort(numbers.data_, numbers.size(), sizeof(int), sortNumbers);

	for (size_t i = 0; i < numbers.size(); i++) {
		fprintf(fileout, "%d\n", numbers.at(i));
	}

	fclose(fileout);
	// Automatic resource management
	return 0; // when we exit the scope of the variable the compiler will call the destructor for us

}