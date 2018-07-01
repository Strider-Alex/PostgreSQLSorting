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
	return strcmp(*((SORT_TYPE*)a), *((SORT_TYPE*)b));
#else
	return *((SORT_TYPE*)a) - *((SORT_TYPE*)b);
#endif
}
int cmp_reverse(const void *a, const void *b) {
#if TYPE_CODE == 2
	return strcmp(*((SORT_TYPE*)b), *((SORT_TYPE*)a));
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

/* generate random string */
static char *rand_string(char *str, size_t size){
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (size) {
		--size;
		for (size_t n = 0; n < size; n++) {
			int key = rand() % (int)(sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
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
static void generateTestData(SORT_TYPE* a, enum Pattern pattern, int size) {

	//open file to write
	char fname[30];
	makeFileName(fname, pattern, size);
	FILE *f = fopen(fname, "w");
	if (!f) {
		printf("Error writing to file: %s\n", fname);
		exit(1);
	}

	//generate data
	for (int i = 0; i < size; i++) {
#if TYPE_CODE == 0 || TYPE_CODE == 1
		a[i] = random_int(MAX_INT);
#elif TYPE_CODE == 2
		a[i] = malloc(sizeof(char)*MAX_STR_LEN);
		rand_string(a[i], MAX_STR_LEN);
#endif
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
#if TYPE_CODE == 0 || TYPE_CODE == 1
		fprintf(f, "%d ", a[i]);
#elif TYPE_CODE == 2
		fprintf(f, "%s ", a[i]);
#endif
	}

	/* close file */
	fclose(f);
}

/* read array data from disk */
static void readTestData(SORT_TYPE* a, enum Pattern pattern, int size) {
	// open file
	char fname[30];
	makeFileName(fname, pattern, size);
	FILE *f = fopen(fname, "r");

	// read data
	for (int i = 0; i < size; i++) {
#if  TYPE_CODE == 0 || TYPE_CODE == 1
		fscanf(f,"%d", &a[i]);
#elif TYPE_CODE == 2
		a[i] = malloc(sizeof(char)*MAX_STR_LEN);
		fscanf(f, "%s", a[i]);
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
	clock_t start_t, end_t;

	printf("%s:\n", name);

	for (enum Pattern p = SORTED; p <= MOSTLY_REVERSED; p++) {
		for (int n = min; n <= max; n *= 10) {
			char fname[30];
			makeFileName(fname, p, n);
			if (!file_exist(fname)) {
				generateTestData(copy, p, n);
			}
			else {
				readTestData(copy, p, n);
			}
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

#if TYPE_CODE == 2
			// free strings
			for (int i = 0; i < n; i++) {
				free(copy[i]);
			}
#endif
		}
	}
	puts("\n");
}

void test() {
	SORT_TYPE a[MAX_N];
	SORT_TYPE copy[MAX_N];

	// pg intro sort
	testSorting(pg_qsort, a, copy, MIN_N, MAX_N, REPEAT, "pg intro sort");

	// pg qsort
	testSorting(old_pg_qsort, a, copy, MIN_N, MAX_N, REPEAT, "pg_qsort");
}
