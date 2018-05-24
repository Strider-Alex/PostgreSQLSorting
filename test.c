#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include "pg_qsort.h"

/* TYPE CODE: 
 * 0 - int, 1 - char, 2 - string
 * BENCHMARK MODE:
 * 0 - count number of comparisons, dont print array, count CPU clks
 * 1 - dont print array, count CPU clks
 */
#define TYPE_CODE 1
#define BENCHMARK_MODE 1

#if TYPE_CODE == 0
#define SORT_NAME int
#define SORT_TYPE int
#elif TYPE_CODE == 1
#define SORT_NAME char
#define SORT_TYPE char
#elif TYPE_CODE == 2
typedef struct StringStruct {
	char data[10];
} string;
#define SORT_NAME string
#define SORT_TYPE string
#endif

/* You can redefine the comparison operator.
The default is
#define SORT_CMP(x, y)  ((x) < (y) ? -1 : ((x) == (y) ? 0 : 1))
but the one below is often faster for integer types.
*/
#if TYPE_CODE == 2
#define _SORT_CMP(x, y) (strcmp((x).data,(y).data))
#else
#define _SORT_CMP(x, y) (x - y)
#endif

int CMP_COUNT = 0; // number of comparisons

#if BENCHMARK_MODE == 0
#define SORT_CMP(x, y)((CMP_COUNT++,_SORT_CMP(x,y)))
#else
#define SORT_CMP(x, y)(_SORT_CMP(x, y))
#endif

#include "sort.h"

#define ARRAYSIZE 2000000
#define MAX_ELEMENT 5000000


int cmp(const void *a, const void *b) {
#if BENCHMARK_MODE == 0
	CMP_COUNT++;
#endif
#if TYPE_CODE == 2
	return strcmp((*((SORT_TYPE*)a)).data, (*((SORT_TYPE*)b)).data);
#else
	return *((SORT_TYPE*)a) - *((SORT_TYPE*)b);
#endif
}

void printElements(SORT_TYPE* a,size_t size) {
	for (int i = 0; i < size; i++) {
#if TYPE_CODE == 0
		printf("%d ", a[i]);
#elif TYPE_CODE == 1
		printf("%c ", a[i]);
#elif TYPE_CODE == 2
		printf("%s ", a[i].data);
#endif
	}
}

void initArray(SORT_TYPE* a) {
	for (int i = 0; i < ARRAYSIZE; i++) {
#if TYPE_CODE == 0
		a[i] = rand() % MAX_ELEMENT;
#elif TYPE_CODE == 1
		a[i] = rand() % 26 + 'a';
#elif TYPE_CODE == 2
		_itoa_s(rand() % MAX_ELEMENT,a[i].data,10, 10);
#endif
	}
}

void testSorting(void (*sort)(SORT_TYPE*,size_t), SORT_TYPE* a, SORT_TYPE* copy, char* name) {
	clock_t start_t, end_t, total_t;
	
	printf("%s:\n", name);
	memcpy(a, copy, ARRAYSIZE * sizeof(SORT_TYPE));
	CMP_COUNT = 0;

	start_t = clock();
	sort(a, ARRAYSIZE);
	end_t = clock();
	total_t = (double)(end_t - start_t);
	printf("CPU clks: %d\n", total_t);

#if BENCHMARK_MODE > 1
	printElements(a, ARRAYSIZE);
#endif
#if BENCHMARK_MODE == 0
	printf("#comparisons: %d", CMP_COUNT);
#endif

	puts("\n");
}

void testSortingWithCompare(void(*sort)(SORT_TYPE*, size_t,
	int(*)(const void*,const void*)), SORT_TYPE* a, SORT_TYPE* copy, char* name) {
	clock_t start_t, end_t, total_t;
	
	printf("%s:\n", name);
	memcpy(a, copy, ARRAYSIZE * sizeof(SORT_TYPE));
	CMP_COUNT = 0;

	start_t = clock();
	sort(a, ARRAYSIZE,sizeof(SORT_TYPE),cmp);
	end_t = clock();
	total_t = (double)(end_t - start_t);
	printf("CPU clks: %d\n", total_t);

#if BENCHMARK_MODE > 1
	printElements(a, ARRAYSIZE);
#endif
#if BENCHMARK_MODE == 0
	printf("#comparisons: %d", CMP_COUNT);
#endif
	puts("\n");
}

int main() {
	SORT_TYPE a[ARRAYSIZE], copy[ARRAYSIZE];
	srand((unsigned)time(NULL));

	initArray(a);

	memcpy(copy, a, ARRAYSIZE * sizeof(SORT_TYPE));

	// c standard quick sort
	testSortingWithCompare(qsort, a, copy, "c standard quick sort");

	// pg quick sort
	testSortingWithCompare(pg_qsort, a, copy, "pg qsort");

	// wekipedia version quick sort
	testSorting(QUICK_SORT,a,copy,"wekipedia version quick sort");

	// tim sort
	testSorting(TIM_SORT, a, copy, "tim sort");

	// intro sort
	testSorting(INTRO_SORT, a, copy, "intro sort");

	// dual-pivot quick sort
	testSorting(DUAL_PIVOT_QUICK_SORT, a, copy, "dual-pivot quick sort");

#if TYPE_CODE == 0
	// radix sort
	testSorting(RADIX_SORT, a, copy, "radix sort");
#endif

	system("pause");
	return 0;
}