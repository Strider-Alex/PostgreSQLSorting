#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define TYPE_CODE 1

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
#define SORT_CMP(x, y) (strcmp((x).data,(y).data))
#else
#define SORT_CMP(x, y) (x - y)
#endif

#include "sort.h"

#define ARRAYSIZE 100
#define MAX_ELEMENT 1000

int cmp(const void *a, const void *b) {
#if TYPE_CODE == 2
	return strcmp((*((SORT_TYPE*)a)).data, (*((SORT_TYPE*)b)).data);
#else
	return *((SORT_TYPE*)a) - *((SORT_TYPE*)b);
#endif
}

void printElement(SORT_TYPE e) {
#if TYPE_CODE == 0
	printf("%d ", e);
#elif TYPE_CODE == 1
	printf("%c ", e);
#elif TYPE_CODE == 2
	printf("%s ", e.data);
#endif
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

int main() {
	SORT_TYPE a[ARRAYSIZE], copy[ARRAYSIZE];
	srand(0);

	initArray(a);

	memcpy(copy, a, ARRAYSIZE * sizeof(SORT_TYPE));

	//c standard quick sort
	qsort(a, ARRAYSIZE, sizeof(SORT_TYPE), cmp);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printElement(a[i]);
	}
	puts("\n");

	//wekipedia version quick sort
	memcpy(a, copy, ARRAYSIZE * sizeof(SORT_TYPE));
	QUICK_SORT(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printElement(a[i]);
	}
	puts("\n");

	// tim sort
	memcpy(a, copy, ARRAYSIZE * sizeof(SORT_TYPE));
	TIM_SORT(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printElement(a[i]);
	}
	puts("\n");

	// intro sort
	memcpy(a, copy, ARRAYSIZE * sizeof(SORT_TYPE));
	INTRO_SORT(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printElement(a[i]);
	}
	puts("\n");

	// dual-pivot quick sort
	memcpy(a, copy, ARRAYSIZE * sizeof(SORT_TYPE));
	DUAL_PIVOT_QUICK_SORT(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printElement(a[i]);
	}
	puts("\n");

	system("pause");
	return 0;
}