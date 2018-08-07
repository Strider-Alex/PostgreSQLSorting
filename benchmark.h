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
//high cardinality int
#define SORT_TYPE int
#define MAX_INT 1000000000
#define INT_GEN
#elif TYPE_CODE == 1
//characters
#define SORT_TYPE char
#define MAX_INT 256
#define INT_GEN
#elif TYPE_CODE == 2
//high cardinality string
#define SORT_TYPE char*
#define STR_GEN
#define MAX_STR_LEN 10
#elif TYPE_CODE == 3
//high cardinality double
#define SORT_TYPE double
#define MAX_INT 100000000
#define DOU_GEN
#elif TYPE_CODE == 4
//low cardinality int
#define SORT_TYPE int
#define MAX_INT 10000
#define INT_GEN
#elif TYPE_CODE == 5
//low cardinality int
#define SORT_TYPE char*
#define STR_GEN
#define MAX_STR_LEN 2
#elif TYPE_CODE == 6
//high cardinality double
#define SORT_TYPE double
#define MAX_INT 10000
#define DOU_GEN
#endif


#if TYPE_CODE == 2
#define SORT_CMP(x, y) (strcmp(x,y))
#else
#define SORT_CMP(x, y) (x - y)
#endif

#define BIN_NUM 100
#define MIN_N 100000
#define MAX_N 10000000
#define REPEAT 5
//#define PRINTOUT

enum Pattern { SORTED, UNSORTED, REVERSED, MOSTLY_SORTED, MOSTLY_REVERSED, KILLER };

void test();
void testSorting(void(*sort)(void*, size_t, size_t, int(*)(const void*, const void*)),
	SORT_TYPE* a, SORT_TYPE* copy, int min, int max, int rounds, char* name);
