#include<stdio.h>
#include<stdlib.h>

#define SORT_NAME int
#define SORT_TYPE int
/* You can redefine the comparison operator.
The default is
#define SORT_CMP(x, y)  ((x) < (y) ? -1 : ((x) == (y) ? 0 : 1))
but the one below is often faster for integer types.
*/
#define SORT_CMP(x, y) (x - y)
#include "sort.h"

#define ARRAYSIZE 100
#define MAX_ELEMENT 1000

int int_cmp(const void *a, const void *b) {
	return *((int*)a) - *((int*)b);
}

int main() {
	int a[ARRAYSIZE];
	srand(0);

	//c standard quick sort
	for (int i = 0; i < ARRAYSIZE; i++) {
		a[i] = rand() % MAX_ELEMENT;
	}
	qsort(a, ARRAYSIZE, sizeof(SORT_TYPE), int_cmp);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printf("%d ", a[i]);
	}
	puts("\n");

	//wekipedia version quick sort
	for (int i = 0; i < ARRAYSIZE; i++) {
		a[i] = rand() % MAX_ELEMENT;
	}
	int_quick_sort(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printf("%d ", a[i]);
	}
	puts("\n");

	// tim sort
	for (int i = 0; i < ARRAYSIZE; i++) {
		a[i] = rand() % MAX_ELEMENT;
	}
	int_tim_sort(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printf("%d ", a[i]);
	}
	puts("\n");

	// intro sort
	for (int i = 0; i < ARRAYSIZE; i++) {
		a[i] = rand() % MAX_ELEMENT;
	}
	int_intro_sort(a, ARRAYSIZE);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printf("%d ", a[i]);
	}
	puts("\n");

	system("pause");
	return 0;
}