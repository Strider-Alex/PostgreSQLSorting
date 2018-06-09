#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "benchmark.h"
#include "qsort.h"
/*
Sorting Benchmark
Array Patterns:
sorted, unsorted(random), mostly sorted, reversed, mostly reversed
Data Type:
int(0), char(1), string(2), struct
*/

/* compare functions, used for qsort */
int cmp(const void *a, const void *b) {
#if TYPE_CODE == 2
	return strcmp((*((SORT_TYPE*)a)).data, (*((SORT_TYPE*)b)).data);
#else
	return *((SORT_TYPE*)a) - *((SORT_TYPE*)b);
#endif
}
int cmp_reverse(const void *a, const void *b) {
#if TYPE_CODE == 2
	return strcmp((*((SORT_TYPE*)b)).data, (*((SORT_TYPE*)a)).data);
#else
	return *((SORT_TYPE*)b) - *((SORT_TYPE*)a);
#endif
}

/* Only tested using Visual C++ 14.1 on Windows, where RAND_MAX is 0x7fff
Bit shifting is needed to generate random integer larger than RAND_MAX
*/
static int random_int(int max) {
	static bool init = false;
	if (!init) {
		srand((unsigned)time(NULL));
		init = true;
	}
	long long num = rand();
	if (RAND_MAX < max) {
		num = (num << 16) + rand();
	}
	return num % max;
}

/* concat file name based on type, array pattern and size */
static void makeFileName(char* output, enum Pattern pattern, int size) {
	sprintf(output, "td_%d_%d_%d.txt", TYPE_CODE, pattern, size);
}

/* swap block of data, tmp is an array passed in by the caller
to store temparary data.
*/
static void swapBlock(SORT_TYPE* a, SORT_TYPE* b, int len, SORT_TYPE* tmp) {
	int size = len * sizeof(SORT_TYPE);
	memcpy(tmp, a, size);
	memcpy(a, b, size);
	memcpy(b, tmp, size);
}

/* shuffle an array using bins of size binLen
used to create mostly sorted and mostly reversed arrays
binLen must divide size
*/
static void shuffleArray(SORT_TYPE* a, int size, int binLen) {
	SORT_TYPE* tmp = (SORT_TYPE*)malloc(sizeof(SORT_TYPE)*binLen);
	for (int i = size / binLen - 1; i > 0; i--) {
		// Pick a random index from 0 to i
		int j = random_int(i + 1);
		// Swap arr[i] with the element at random index
		if (i != j) {
			swapBlock(&a[i*binLen], &a[j*binLen], binLen, tmp);
		}
	}
	free(tmp);
}

/* function to generate test data and write to disk */
static void generateTestData(enum Pattern pattern, int size) {

	//open file to write
	char fname[30];
	makeFileName(fname, pattern, size);
	FILE *f = fopen(fname, "w");
	if (!f) {
		printf("Error writing to file: %s\n", fname);
		exit(1);
	}

	//generate data
	SORT_TYPE* a = (SORT_TYPE*)malloc(sizeof(SORT_TYPE)*size);
	for (int i = 0; i < size; i++) {
		a[i] = random_int(MAX_ELEMENT);
	}

	//deal with pattern
	switch (pattern) {
	case UNSORTED:
		break;
	case SORTED:
		qsort(a, size, sizeof(SORT_TYPE), cmp);
		break;
	case REVERSED:
		qsort(a, size, sizeof(SORT_TYPE), cmp_reverse);
		break;
	case MOSTLY_SORTED:
		qsort(a, size, sizeof(SORT_TYPE), cmp);
		shuffleArray(a, size, size / BIN_NUM);
		break;
	case MOSTLY_REVERSED:
		qsort(a, size, sizeof(SORT_TYPE), cmp_reverse);
		shuffleArray(a, size, size / BIN_NUM);
		break;
	default:
		break;
	}

	/* write array to file */
	for (int i = 0; i < size; i++) {
#if TYPE_CODE == 0
		fprintf(f, "%d ", a[i]);
#endif
	}

	/* close file */
	fclose(f);
}

/* read array data from disk */
static void readTestData(int* a, enum Pattern pattern, int size) {
	// open file
	char fname[30];
	makeFileName(fname, pattern, size);
	FILE *f = fopen(fname, "r");

	// read data
	for (int i = 0; i < size; i++) {
#if TYPE_CODE == 0
		fscanf(f,"%d", &a[i]);
#endif
	}

	//close file
	fclose(f);
}

static int file_exist(char *filename)
{
	struct stat   buffer;
	return (stat(filename, &buffer) == 0);
}

void testSorting(void(*sort)(void*, size_t,size_t,int(*)(const void*,const void*)),
	SORT_TYPE* a, SORT_TYPE* copy, int min, int max, int rounds, char* name) {
	clock_t start_t, end_t, total_t;

	printf("%s:\n", name);

	for (enum Pattern p = SORTED; p <= MOSTLY_REVERSED; p++) {
		for (int n = min; n <= max; n *= 10) {
			char fname[30];
			makeFileName(fname, p, n);
			if (!file_exist(fname)) {
				generateTestData(p, n);
			}
			readTestData(copy, p, n);
			int ticksum = 0;
			for (int r = 0; r < rounds; r++) {
				memcpy(a, copy, n * sizeof(SORT_TYPE));
				start_t = clock();
				sort(a, n, sizeof(SORT_TYPE),cmp);
				end_t = clock();
				ticksum += end_t - start_t;
			}
			bool correct = true;
#ifdef PRINTOUT
			for (int i = 0; i < n - 1; i++) {
				printf("%d ", a[i]);
			}
#endif
			for (int i = 0; i < n - 1; i++) {
				if (SORT_CMP(a[i], a[i + 1]) > 0) {
					correct = false;
					break;
				}
			}
			printf("Pattern %d, n = %d, correct: %d, CPU clks: %lf\n", p, n, correct, 1.0*ticksum / rounds);
		}
	}
	puts("\n");
}

void test() {
	int a[MAX_N], copy[MAX_N];

	// new pg qsort
	testSorting(heap_sort, a, copy, MIN_N, MAX_N, 100, "new heap sort");

	// new pg qsort
	testSorting(pg_qsort, a, copy, MIN_N, MAX_N, 100, "new pg_qsort");

	// intro sort
	testSorting(intro_sort, a, copy, MIN_N, MAX_N, 100, "intro sort");

	// new qsort
	testSorting(quick_sort, a, copy, MIN_N, MAX_N, 100, "new quick sort");

	// tim_sort
	testSorting(tim_sort, a, copy, MIN_N, MAX_N, 100, "new tim sort");

	// new dual-pivot qsort
	testSorting(dual_pivot_quick_sort, a, copy, MIN_N, MAX_N, 100, "new dual-pivot quick sort");
}