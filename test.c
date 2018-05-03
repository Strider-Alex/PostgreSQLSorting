#include<stdio.h>
#include<stdlib.h>
#include"introsort.h"

#define ARRAYSIZE 100

int int_cmp(const void *a, const void *b) {
	return *((int*)a) - *((int*)b);
}

int main() {
	int a[ARRAYSIZE];
	srand(0);
	for (int i = 0; i < ARRAYSIZE; i++) {
		a[i] = rand()%1000;
	}
	introsort(a, 100, sizeof(int), int_cmp);
	for (int i = 0; i < ARRAYSIZE; i++) {
		printf("%d ", a[i]);
	}
	
	return 0;
}