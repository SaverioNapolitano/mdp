#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int sortNumbers(const void* a, const void* b) {
	const int* x = a;
	const int* y = b;

	return (*x > *y) - (*x < *y);
}

typedef struct vector { // it's at least 24 bytes since we are compiling for x64 architecture (compiler might put some spaces in between or after the end to align in memory, it never puts space before)
	size_t size, capacity;
	int* data;
} vector;

// constructor: function that initializes the object
void vector_constructor(vector* this) {
	this->size = 0;
	this->capacity = 10;
	this->data = malloc(this->capacity * sizeof(int));
}

void vector_destructor(vector* this) { // vector is a container that points to some resources
	free(this->data);
}

void vector_push_back(vector* this, int num) {
	if (this->size == this->capacity) {
		this->capacity *= 2;
		this->data = realloc(this->data, this->capacity * sizeof(int));;
	}
	this->data[this->size] = num;
	this->size++;
}

size_t vector_size(const vector* this) { // vector const *this is equivalent
	return this->size;
}

int vector_at(const vector* this, size_t index) {
	assert(index < this->size); // If you compile in debug mode it does the check, it disappears in release mode
	return this->data[index];
}

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

	vector numbers; // definition of a variable
	vector_constructor(&numbers); // initialization, we need the address because the function is going to modify the variable

	int num;
	while (fscanf(filein, "%d", &num) == 1) {
		vector_push_back(&numbers, num);
	}

	fclose(filein);

	qsort(numbers.data, numbers.size, sizeof(int), sortNumbers);

	for (size_t i = 0; i < vector_size(&numbers); i++) {
		fprintf(fileout, "%d\n", vector_at(&numbers, i));
	}

	fclose(fileout);
	vector_destructor(&numbers);
	return 0;

}