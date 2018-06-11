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
#elif TYPE_CODE == 1
#define SORT_TYPE char
#define SORT_NAME char
#elif TYPE_CODE == 2
#define SORT_TYPE char*
#define SORT_NAME string
#endif


#if TYPE_CODE == 2
#define SORT_CMP(x, y) (strcmp(x,y))
#else
#define SORT_CMP(x, y) (x - y)
#endif

#define MAX_INT 1000000000
#define MAX_STR_LEN 20
#define BIN_NUM 100
#define MIN_N 1000
#define MAX_N 1000000
//#define PRINTOUT

enum Pattern { SORTED, UNSORTED, REVERSED, MOSTLY_SORTED, MOSTLY_REVERSED };

void test();
void testSorting(void(*sort)(void*, size_t, size_t, int(*)(const void*, const void*)),
	SORT_TYPE* a, SORT_TYPE* copy, int min, int max, int rounds, char* name);
