#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

int sortNumbers(const void* a, const void* b) {
	const int* x = a;
	const int* y = b;

	return (*x > *y) - (*x < *y);
}

int main(int argc, char** argv) {
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

	int number;
	int ret;
	size_t size = 1;
	int* numbers = malloc(size * sizeof(int));

	while ((ret = fscanf(filein, "%i", &number)) == 1) { // fscanf SKIPS WHITE SPACES
		numbers[size - 1] = number;
		size++;
		numbers = realloc(numbers, size * sizeof(int));
	}
	size--;
	numbers = realloc(numbers, size * sizeof(int));
	qsort(numbers, size, sizeof(int), sortNumbers);

	for (int i = 0; i < size; i++) {
		number = numbers[i];
		fprintf(fileout, "%i\n", number);
	}

	fclose(filein);
	fclose(fileout);
	free(numbers);
	return 0;

}