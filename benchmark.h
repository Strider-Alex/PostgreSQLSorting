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

#if TYPE_CODE == 2
#define SORT_CMP(x, y) (strcmp((x).data,(y).data))
#else
#define SORT_CMP(x, y) (x - y)
#endif

#define MAX_ELEMENT 1000000000
#define BIN_NUM 100

enum Pattern { SORTED, UNSORTED, REVERSED, MOSTLY_SORTED, MOSTLY_REVERSED };

void test();