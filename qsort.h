#ifndef MAX_ES
#define MAX_ES 16
#endif

#ifndef INSERTION_THRESHOLD
#define INSERTION_THRESHOLD 16U
#endif

#ifndef TIM_SORT_STACK_SIZE
#define TIM_SORT_STACK_SIZE 128
#endif

#ifndef RADIX_SORT_BASE
#define RADIX_SORT_BASE 10
#endif

#ifndef CLZ
#ifdef __GNUC__
#define CLZ __builtin_clzll
#else

#include <stdint.h>

static int clzll(uint64_t);

/* adapted from Hacker's Delight */
static int clzll(uint64_t x) {
	int n;

	if (x == 0) {
		return 64;
	}

	n = 0;

	if (x <= 0x00000000FFFFFFFFL) {
		n = n + 32;
		x = x << 32;
	}

	if (x <= 0x0000FFFFFFFFFFFFL) {
		n = n + 16;
		x = x << 16;
	}

	if (x <= 0x00FFFFFFFFFFFFFFL) {
		n = n + 8;
		x = x << 8;
	}

	if (x <= 0x0FFFFFFFFFFFFFFFL) {
		n = n + 4;
		x = x << 4;
	}

	if (x <= 0x3FFFFFFFFFFFFFFFL) {
		n = n + 2;
		x = x << 2;
	}

	if (x <= 0x7FFFFFFFFFFFFFFFL) {
		n = n + 1;
	}

	return n;
}

#define CLZ clzll
#endif
#endif

void quick_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *));
void dual_pivot_quick_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *));
void pg_qsort(void *a, size_t n, size_t es, int(*cmp) (const void *, const void *));
void tim_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *));