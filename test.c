#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

#include "benchmark.h"

int main() {
	freopen("result.csv", "w", stdout);
	test();
	return 0;
}