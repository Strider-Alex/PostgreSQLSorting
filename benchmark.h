#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/*
Sorting Benchmark
Array Patterns:
sorted, unsorted(random), mostly sorted, reversed, mostly reversed
Data Type:
int(0), char(1), string(2), struct
*/

#ifndef TYPE_CODE
#define TYPE_CODE 0
#endif

#if TYPE_CODE == 0
#define SORT_TYPE int
#define SORT_NAME int
#endif

#define MAX_ELEMENT 1000000000
#define BIN_NUM 100

#include "sort.h"

enum Pattern { SORTED, UNSORTED, REVERSED, MOSTLY_SORTED, MOSTLY_REVERSED };

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
static void makeFileName(char* output, enum Pattern pattern,int size) {
	sprintf(output, "td_%d_%d_%d.txt", TYPE_CODE, pattern,size);
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
	for (int i = size/binLen - 1; i > 0; i--){
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
	if (f == NULL){
		printf("Error writing to file: %s\n",fname);
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

