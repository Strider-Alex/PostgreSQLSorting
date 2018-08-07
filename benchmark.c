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
sorted, unsorted(random), mostly sorted, reversed, mostly reversed, killer
Data Type:
int(0), char(1), string(2), struct
*/

/* compare functions, used for qsort */
int cmp(const void *a, const void *b) {
#ifdef STR_GEN
	return strcmp(*((SORT_TYPE*)a), *((SORT_TYPE*)b));
#else
	return *((SORT_TYPE*)a) - *((SORT_TYPE*)b);
#endif
}
int cmp_reverse(const void *a, const void *b) {
#ifdef STR_GEN
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

static char *get_max_str(char *str, size_t size) {
	for (int i = 0; i < size; i++) {
		str[i] = 'z' + 1;
	}
	str[size] = '\0';
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

/* generate qsort killer sequence using a sorted array */
struct SORT_TYPE_WITH_POS {
	SORT_TYPE val;
	int pos;
};

typedef struct SORT_TYPE_WITH_POS SORT_POS;

/* swap function for SORT_POS */
void swapPos(SORT_POS* a, SORT_POS* b) {
	SORT_POS c = *b;
	*b = *a;
	*a = c;
	return;
}
void swapPosVec(SORT_POS* a, SORT_POS* b, int n) {
	for (int i = 0; i < n; i++) {
		SORT_POS c = *(b + i);
		*(b + i) = *(a + i);
		*(a + i) = c;
	}
	return;
}

static SORT_TYPE upperBound() {
#ifdef INT_GEN
	return MAX_INT;
#elif defined STR_GEN
	static char str[MAX_STR_LEN + 1];
	return get_max_str(str, MAX_STR_LEN);
#elif defined DOU_GEN
	return (double)MAX_INT;
#endif
}

static int safeSet(SORT_POS* a, int i, SORT_TYPE val) {
	if (a[i].val == upperBound()) {
		a[i].val = val;
		return 1;
	}
	return 0;
}
static void generate_killer_recursive(SORT_POS *a, SORT_TYPE* sorted, size_t n, int* pmin, int* pmax)
{
	SORT_POS	   *pa,
		*pb,
		*pc,
		*pd,
		*pl,
		*pm,
		*pn;
	size_t		d1,
		d2;
	int			r;

loop:
	if (n < 7)
	{
		for (int i = 0; i < n; i++) {
			if (safeSet(a, i, sorted[*pmax])) {
				*pmax -= 1;
			}	
		}
		return;
	}
	pm = a + (n / 2);
	if (n > 7)
	{
		pl = a;
		pn = a + (n - 1);
		if (n > 40)
		{
			size_t d = (n / 8);
			int indexes[] = { 0,d,2 * d,pm - a - d,pm - a,pm - a + d,pn - a - 2 * d,pn - a - d,pn - a };
			for (int i = 0; i < 9; i++) {
				if (safeSet(a, indexes[i], sorted[*pmin])) {
					*pmin += 1;
				}
			}
		}
		else {
			int indexes[] = { 0,pm - a,pn - a };
			for (int i = 0; i < 3; i++) {
				if (safeSet(a, indexes[i], sorted[*pmin])) {
					*pmin += 1;
				}
			}
		}
	}
	swapPos(a, pm);
	pa = pb = a + 1;
	pc = pd = a + (n - 1);
	for (;;)
	{
		while (pb <= pc && (r = cmp(pb, a)) <= 0)
		{
			if (r == 0)
			{
				swapPos(pa, pb);
				pa ++;
			}
			pb ++;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0)
		{
			if (r == 0)
			{
				swapPos(pc, pd);
				pd --;
			}
			pc --;
		}
		if (pb > pc)
			break;
		swapPos(pb, pc);
		pb ++;
		pc --;
	}
	pn = a + n;
	d1 = min(pa - a, pb - pa);
	swapPosVec(a, pb - d1, d1);
	d1 = min(pd - pc, pn - pd - 1);
	swapPosVec(pb, pn - d1, d1);
	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2)
	{
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > 1)
			generate_killer_recursive(a, sorted, d1, pmin, pmax);
		if (d2 > 1)
		{
			a = pn - d2;
			n = d2;
			goto loop;
		}
	}
	else
	{
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > 1)
			generate_killer_recursive(pn - d2, sorted, d2, pmin, pmax);
		if (d1 > 1)
		{
			n = d1;
			goto loop;
		}
	}
}

static void generateMedKiller(SORT_TYPE* a,  int size) {
	int imin = 0, imax = size - 1;
	SORT_POS* tmp = (SORT_POS*)malloc(sizeof(SORT_POS)*size);
	for (int i = 0; i < size; i++) {
		tmp[i].pos = i;
		tmp[i].val = upperBound();
	}
	generate_killer_recursive(tmp, a, size, &imin, &imax);
	for (int i = 0; i < size; i++) {
		if (imax <= imin) {
			break;
		}
		if (safeSet(tmp, i, a[imax])) {
			imax -= 1;
		}
	}
	for (int i = 0; i < size; i++) {
		a[tmp[i].pos] = tmp[i].val;
	}
	free(tmp);
	return;
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
#ifdef INT_GEN
		a[i] = random_int(MAX_INT);
#elif defined STR_GEN
		a[i] = malloc(sizeof(char)*MAX_STR_LEN);
		rand_string(a[i], MAX_STR_LEN);
#elif defined DOU_GEN
		a[i] = (double)random_int(MAX_INT);
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
	case KILLER:
		qsort(a, size, sizeof(SORT_TYPE), cmp);
		generateMedKiller(a, size);
		break;
	default:
		break;
	}

	/* write array to file */
	for (int i = 0; i < size; i++) {
#ifdef INT_GEN
		fprintf(f, "%d ", a[i]);
#elif defined STR_GEN
		fprintf(f, "%s ", a[i]);
#elif defined DOU_GEN
		fprintf(f, "%lf ", a[i]);
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
#ifdef INT_GEN
		fscanf(f,"%d", &a[i]);
#elif defined STR_GEN
		a[i] = malloc(sizeof(char)*MAX_STR_LEN);
		fscanf(f, "%s", a[i]);
#elif defined DOU_GEN
		fscanf(f, "%lf", &a[i]);
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

	for (enum Pattern p = SORTED; p <= KILLER; p++) {
		for (int n = min; n <= max; n *= 10) {
			if (p == KILLER && n > max / 10) {
				break;
			}
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

			printf("%s,%d,%d,%d,%.3lf\n", name, p, n, correct, 1.0*ticksum / rounds);

#ifdef STR_GEN
			// free strings
			for (int i = 0; i < n; i++) {
				free(copy[i]);
			}
#endif
		}
	}
}

void test() {
	SORT_TYPE a[MAX_N];
	SORT_TYPE copy[MAX_N];

	printf("sorting routine,pattern,n,correct,time(CPU clock ticks)\n");

	testSorting(heap_sort_wrapper, a, copy, MIN_N, MAX_N, REPEAT, "heap sort");

	testSorting(tim_sort, a, copy, MIN_N, MAX_N, REPEAT, "tim sort");

	testSorting(dual_pivot_quick_sort, a, copy, MIN_N, MAX_N, REPEAT, "dual pivot quick sort");

	//testSorting(quick_sort, a, copy, MIN_N, MAX_N, REPEAT, "median of 3 quick sort");

#ifdef INT_GEN
	testSorting(radix_sort, a, copy, MIN_N, MAX_N, REPEAT, "radix sort");
#endif

	// pg intro sort
	testSorting(pg_qsort, a, copy, MIN_N, MAX_N, REPEAT, "pg intro sort");

	// pg intro sort once
	testSorting(pg_qsort_once, a, copy, MIN_N, MAX_N, REPEAT, "pg intro sort - 1 preordered check");

	// pg qsort
	testSorting(rand_pg_qsort, a, copy, MIN_N, MAX_N, REPEAT, "rand pg_qsort");

	// pg qsort
	testSorting(old_pg_qsort, a, copy, MIN_N, MAX_N, REPEAT, "pg_qsort");
}
