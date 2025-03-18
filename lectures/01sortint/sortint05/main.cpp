#define _CRT_SECURE_NO_WARNINGS
//#include <stdio.h>
//#include <stdlib.h>
//#include <assert.h>
// The right way to include C headers in C++ programs
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <crtdbg.h> //it slightly changes the definition of some memory allocation functions such that in debug mode it keeps track of all the memory allocation that has been done
#include <utility> 

int sortNumbers(const void* a, const void* b) {
	const int* x = (const int*)a;
	const int* y = (const int*)b;

	return (*x > *y) - (*x < *y);
}
// Copy and swap idiom -> how to avoid writing two separated assignment operators

// By defining the functions inside the definition of the struct the compiler knows they will operate on it
// namespace: part of the name of the function given by the compiler (can be omitted or explicitly written)
// C++ will automatically put for every method a pointer to the object (no need to write it), we can use this implicitly
// this is a vector * 
// We can use just the name of the structure even without typedef 
//If you need a destructor, copy constructor or assignment operator (one of them) then you need all of them (the fourth is the default constructor) -> Rule of 4
//To make a class that manages memory (objects) you need these six (default/copy/move constructor, copy/move assignment, destructor)
template<typename T>
// template makes the struct a model for lots of structs, <meta-parameters> are types for which the struct should be specialized 
struct vector { // it's at least 24 bytes since we are compiling for x64 architecture (compiler might put some spaces in between or after the end to align in memory, never puts space before)
	size_t size_, capacity_; //Google convention: _ after variable name Microsoft: m_ before variable name (m stands for member)
	T* data_;

	//out vector can contain anything that can be copied and assigned


	// for every pointer we must define who is in charge of managing that resource (e.g., who is the owner)
	// constructor: function that initializes the object
	// default constructor
	vector() { // we need to tell the compiler this method is the constructor by naming the method as the class
		printf("default constructor\n");
		size_ = 0; // if there is no ambiguity we can drop this->
		capacity_ = 10;
		//data_ = (T*)malloc(capacity_ * sizeof(T)); // Type system is stricter, we can't use type void freely, we need to convert it explicitly
		data_ = new T[capacity_];
	}
	// copy is another form of construct 
	// copy constructor 
	vector(const vector& other) { // ALWAYS WRITE CONST THEN REMOVE IT IF SOMETHING DOES NOT WORK
		printf("copy constructor\n");
		size_ = other.size_;
		capacity_ = other.capacity_;
		//data_ = (T*)malloc(capacity_ * sizeof(T));
		data_ = new T[capacity_];
		for (size_t i = 0; i < size_; i++) {
			data_[i] = other.data_[i];
		}

	}
	// move constructor (it's like the copy but it's called when the object is temporary or is going to be destroyed right after)
	// const rvalue reference is useless (we want to steal something from it so we need to modify it) 
	vector(vector&& other) { // rvalue reference -> object which is a temporary object or is going to be destroyed shortly after
		printf("copy constructor\n");
		size_ = other.size_;
		capacity_ = other.capacity_;
		data_ = other.data_;
		other.data_ = nullptr; // C++ keyword
	}
	// copy assignment 
	vector& operator=(const vector& other) { //We changed from void copy(...) to vector& operator=(...), vector& is a vector reference
		printf("copy assignment\n");
		if (capacity_ < other.size_) { // to avoid allocating new memory when unnecessary (if this is true this can't be a self assignment
			free(data_); //if we don't do this we get a memory leak
			capacity_ = other.capacity_;
			//data_ = (T*)malloc(capacity_ * sizeof(T));
			data_ = new T[capacity_];
		}
		size_ = other.size_;
		// inefficient in case of self-assignment, but it's unlikely
		for (size_t i = 0; i < size_; i++) {
			data_[i] = other.data_[i];
		}
		// we can't return this since it's a vector * (pointer)
		return *this; // we dereference this in order to return the object pointed to by it since we need it to initialize the reference

	}

	// move assignment 
	vector& operator=(vector&& other) {
		printf("move assignment\n");
		// to avoid allocating new memory when unnecessary (if this is true this can't be a self assignment)
		size_ = other.size_;
		capacity_ = other.capacity_;
		std::swap(data_, other.data_);
		return *this;
	}

	~vector() { // this is the destructor
		printf("destructor()\n");
		//free(data_); //if we allow the copy, we are telling that when we copy also the data will be copied
		delete[] data_;
	}
	// When we have an object as a parameter it will call a copy, it's expensive, if we can we should use reference
	void push_back(const T& num) {
		if (size_ == capacity_) {
			capacity_ *= 2;

			// there is no renew operator because it would be problematic, we have to do it by hand 

			// allocate new memory
			T* tmp = new T[capacity_];

			// copy old data in new memory 
			for (size_t i = 0; i < size_; ++i) {
				tmp[i] = data_[i];
			}
			// delete old memory
			delete[] data_;
			//data_ = (T*)realloc(data_, capacity_ * sizeof(T)); -> nicer and more efficient 
			//point to new memory 
			data_ = tmp;
		}
		data_[size_] = num;
		size_++;
	}

	size_t size() const { // we are communicating to the user that size() is a const method (can be called on a read-only object)
		return size_; // if we try this->size++ it will throw an error 
	}

	const T& at(size_t index) const { // now we don't copy to the vector, we just return a reference to our vector
		assert(index < size_); // If you compile in debug mode it does the check, it disappears in release mode
		return data_[index];
	}

	const T& operator[](size_t index) const {
		return data_[index];
	}

	T& operator[](size_t index) {
		return data_[index];
	}

};
// The idea is to have a pointer that automatically dereferences the object (a reference to something in memory, e.g. a hidden pointer) -> & syntax
// we are not copying anything, we are just making an alias with &v
// we must initialize the reference (can't have NULL references) and we can't change the initial initialization (reference is frozen)
void print(FILE* f, const vector<int>& v) { // if we use vector v numbers and v are two different variables (different addresses in memory) but point to the same location (data)
	for (size_t i = 0; i < v.size(); i++) { // if we use vector v we pass a struct to a function and in C++ (like C) it is shallow copied (just the surface is copied, e.g. the bytes), if we have pointers to something that needs to be duplicated it won't work
		fprintf(f, "%d\n", v[i]); // we can solve it by passing a pointer (*v) instead of v (we are not copying anything but the pointer), but is ugly because we have to use the -> syntax and the & in the main
	}
} //if we use vector v when we get out the vector v is freed and thus the original data is deallocated 
// if we use the reference we will eliminate just the reference 
// WE WANT TO AVOID COPYING AS MUCH AS POSSIBLE

// methods of an object are not inside the object, they are just functions logically related to the object 

vector<int> read(FILE* f) {
	if (f == nullptr) {
		return vector<int>(); // We are creating an empty vector and returning it
	}
	// Named Return Value Optimization (NRVO) technique: if you can try to return only one thing from your function (try not to return different things)
	vector<int> v;

	int num;
	while (fscanf(f, "%d", &num) == 1) {
		v.push_back(num);
	}
	//we are just copying the outer structure
	// the function receives two pointers: one to the file and one to where the vector should be put, so everything is done in the outer memory (as efficient as working with all the code in the main function)
	return v; //a temporary return value would have been created by copying the local variable (that will be destroyed), since we return the same variable the compiler knows that the returned object is always the same (no copy made)
	// What if we can't return just one thing? 
	// We are making a copy not to ruin the original and then destroy the original
	// We want to steal data from an object when it's going to be destroyed right after (the compiler knows we won't be able to use it anymore)
	// We need to code a new constructor that is called whenever the object is temporary (move constructor)
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
};

int main(int argc, char** argv) {
	//size_t x = sizeof(vector);
	{
		// placement new -> version of new when we have already allocated memory and we want to call the constructor there (for optimization purposes)


		//We need something that works like malloc but also calls the constructor (e.g. receives the type of object to be allocated)
		//default constructor can also be called as new widget(), in general new <classname> 
		widget* q = new widget; // new keyword allocates space for one widget and calls the constructor for the widget in that memory 
		//widget* q = (widget *)malloc(sizeof(widget)); // no constructor called (uninitialized memory)
		//widget* q = new widget(6); // we can also pass new some parameters
		//malloc misses the constructor, free misses the destructor -> delete keyword 
		delete q; //before freeing the memory it calls the destructor

		widget* p = new widget[3]; // we allocate space for 3 widgets (it will do a for loop three times calling the constructor for each of them)
		delete[] p; //delete p without [] won't work -> delete[] is a single keyword

		//We need one keyword to delete an object and one keyword for deleting a pointer to some objects 
		//because when we delete a pointer we need to do a loop for every object it points to but we need to know how many objects it points to
		//the compiler stores the number of elements allocated into memory when it's more than 1 and it's pointed to by something
		//If we had one single jeyword, if we are allocating multiple single objects we would need to have a pointer also for each of them even if it's just one thing (we save some operations)

		vector<widget> vec;
		// we want the vector to grow (by default it has 10 spaces)
		// Bunch of new allocation (and thus constructors) will be called
		// when we allocate the memory we need to call the constructor for the object that will be put there 
		// malloc() is not the way to go for allocating memory for non POD (plain old data) object since it doesn't know anything about what we are allocating the memory for
		// same holds for free()
		vec.push_back(widget(7));
		vec.push_back(widget(25));
		vec.push_back(widget(123));
		vec.push_back(widget(7));
		vec.push_back(widget(25));
		vec.push_back(widget(123));
		vec.push_back(widget(7));
		vec.push_back(widget(25));
		vec.push_back(widget(123));
		vec.push_back(widget(7));
		vec.push_back(widget(25));
		vec.push_back(widget(123));

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
		//vector numbers; // definition of a variable, the compiler will call the constructor for us automatically

		//numbers = read(filein); //we are copying the local object into our variable (assignment) and then destroy the temporary returned object

		//Return value optimization (RVO) technique: instead of using assignment, if we use initialization the compiler realises the temporary returned object is used to initialize a new variable
		//vector numbers = read(filein); // INITIALIZE INSTEAD OF ASSIGNING EVEY TIME YOU CAN, we avoid the assignment copy
		vector<int> numbers;

		numbers = read(filein);

		fclose(filein);

		//int x = numbers.at(0); //it's an array, we want to use the [] operator
		//int x = numbers[0];
		//numbers[0] = 7; //it doesn't work if we return a const reference 
		//if we eliminate the const at the beginning it works but we are violating the standard (since the operator is defined as const) 
		//we could remove also the const at the end and would be fine but then the print won't work because when we declare a const object we can only call const method
		//we can solve it by duplicating the code (operator overloading), one method to be called when the object is const and one when it is not
		//every function can be written in different ways if the parameters are different, they have different parameters since one has the implicit const vector<T>* this and the other has vector<T>* this
		//Name mangling: real name of the function is its name + a sort of hash of the types of the parameters -> used by the linker to identify different versions of the same function
		vector<double> v;
		v.push_back(3.1);
		v.push_back(4.56);
		v.push_back(0.1); //never use == when comparing floating points numbers (choose a precision and compare them up to that precision since they are not exact)

		qsort(numbers.data_, numbers.size(), sizeof(int), sortNumbers);

		print(fileout, numbers);

		fclose(fileout);
		// Automatic resource management
	} // All the vectors are out of scope thus the destructor will be called when we get here
	_CrtDumpMemoryLeaks(); //It returns the allocated memory which haven't been deallocated up to this point, it dumps them in the Output Window
	return 0; //when we exit the scope of the variable the compiler will call the destructor for us, double free problem if we used a shallow copy

}