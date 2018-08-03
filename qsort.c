#include "qsort.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define Min(X,Y) ((X) < (Y) ? (X) : (Y))

typedef struct {
	size_t alloc;
	char *storage;
} TEMP_STORAGE_T;

typedef struct {
	size_t start;
	size_t length;
} TIM_SORT_RUN_T;


static __inline void* pick(void* a, int i, int es) {
	return (char*)a + i * es;
}

static __inline void assign(void* a, void* b, int es) {
	memcpy(a, b, es);
}

static __inline void swap(void* a, void* b, int es) {
	char* pa = (char*)a, *pb = (char*)b;
	do {
		char tmp = *pa;
		*pa++ = *pb;
		*pb++ = tmp;
	} while (--es > 0);
}

/* Function used to do a binary search for binary insertion sort */
static __inline size_t binary_insertion_sort_find(void *a, const void* x,
	const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	size_t l, c, r;
	char cx[MAX_ES];
	l = 0;
	r = size - 1;
	c = r >> 1;

	/* check for out of bounds at the beginning. */
	if (cmp(x, pick(a, 0, es)) < 0) {
		return 0;
	}
	else if (cmp(x, pick(a, r, es)) > 0) {
		return r;
	}

	assign(cx, pick(a, c, es), es);

	while (1) {
		const int val = cmp(x, cx);

		if (val < 0) {
			if (c - l <= 1) {
				return c;
			}

			r = c;
		}
		else { /* allow = for stability. The binary search favors the right. */
			if (r - c <= 1) {
				return c + 1;
			}

			l = c;
		}

		c = l + ((r - l) >> 1);
		assign(cx, pick(a, c, es), es);
	}
}

/* Binary insertion sort, but knowing that the first "start" entries are sorted.  Used in timsort. */
//TODO: consider pair insertion sort (used in JDK8)
static __inline void binary_insertion_sort_start(void *a, const size_t start, const size_t size,
	const size_t es, int(*cmp) (const void *, const void *)) {
	size_t i;
	char x[MAX_ES];

	for (i = start; i < size; i++) {
		size_t j;
		size_t location;

		/* If this entry is already correct, just move along */
		if (cmp(pick(a, i - 1, es), pick(a, i, es)) <= 0) {
			continue;
		}

		/* Else we need to find the right place, shift everything over, and squeeze in */
		assign(x, pick(a, i, es), es);
		location = binary_insertion_sort_find(a, x, i, es, cmp);

		for (j = i - 1; j >= location; j--) {
			assign(pick(a, j + 1, es), pick(a, j, es), es);
			if (j == 0) { /* check edge case because j is unsigned */
				break;
			}
		}

		assign(pick(a, location, es), x, es);
	}
}

/* Binary insertion sort */
static __inline void binary_insertion_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	/* don't bother sorting an array of size <= 1 */
	if (size <= 1) {
		return;
	}

	binary_insertion_sort_start(a, 1, size, es, cmp);
}

/* Quick sort: based on wikipedia */
static __inline size_t quick_sort_partition(void *a, const size_t left,
	const size_t right, const size_t pivot, const size_t es,
	int(*cmp) (const void *, const void *)) {
	char value[MAX_ES];
	assign(value, pick(a, pivot, es), es);
	size_t index = left;
	size_t i;
	int not_all_same = 0;
	/* move the pivot to the right */
	swap(pick(a, pivot, es), pick(a, right, es), es);

	for (i = left; i < right; i++) {
		int c = cmp(pick(a, i, es), value);
		/* check if everything is all the same */
		not_all_same |= c;

		if (c < 0) {
			swap(pick(a, i, es), pick(a, index, es), es);
			index++;
		}
	}

	swap(pick(a, right, es), pick(a, index, es), es);

	/* avoid degenerate case */
	if (not_all_same == 0) {
		return SIZE_MAX;
	}

	return index;
}

static void quick_sort_recursive(void *a, const size_t left, const size_t right,
	const size_t es, int(*cmp) (const void *, const void *)) {
	size_t pivot;
	size_t new_pivot;

	if (right <= left) {
		return;
	}

	if ((right - left + 1U) < INSERTION_THRESHOLD) {
		binary_insertion_sort(pick(a, left, es), right - left + 1U, es, cmp);
		return;
	}

	pivot = left + ((right - left) >> 1);
	/* this seems to perform worse by a small amount... ? */
	/* pivot = MEDIAN(a, left, pivot, right); */
	new_pivot = quick_sort_partition(a, left, right, pivot, es, cmp);

	/* check for partition all equal */
	if (new_pivot == SIZE_MAX) {
		return;
	}

	quick_sort_recursive(a, left, new_pivot - 1U, es, cmp);
	quick_sort_recursive(a, new_pivot + 1U, right, es, cmp);
}

void quick_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	/* don't bother sorting an array of size 1 */
	if (size <= 1) {
		return;
	}

	quick_sort_recursive(a, 0U, size - 1U, es, cmp);
}



static void dual_pivot_quick_sort_recursive(void *a, const size_t left, const size_t right,
	const size_t es, int(*cmp) (const void *, const void *)) {
	const size_t length = right - left + 1U;

	// Use insertion sort on tiny arrays
	if (length < INSERTION_THRESHOLD) {
		binary_insertion_sort(pick(a, left, es), length, es, cmp);
		return;
	}

	// Inexpensive approximation of length / 7
	size_t seventh = (length >> 3) + (length >> 6) + 1U;

	/*
	* Sort five evenly spaced elements around (and including) the
	* center element in the range. These elements will be used for
	* pivot selection as described below. The choice for spacing
	* these elements was empirically determined to work well on
	* a wide variety of inputs.
	*/
	size_t e3 = (left + right) >> 1; // The midpoint
	size_t e2 = e3 - seventh;
	size_t e1 = e2 - seventh;
	size_t e4 = e3 + seventh;
	size_t e5 = e4 + seventh;

	char t[MAX_ES];

	// Sort these elements using insertion sort
	if (cmp(pick(a, e2, es), pick(a, e1, es))<0) {
		assign(t, pick(a, e2, es), es);
		assign(pick(a, e2, es), pick(a, e1, es), es);
		assign(pick(a, e1, es), t, es);
	}

	if (cmp(pick(a, e3, es), pick(a, e2, es))<0) {
		assign(t, pick(a, e3, es), es);
		assign(pick(a, e3, es), pick(a, e2, es), es);
		assign(pick(a, e2, es), t, es);
		if (cmp(t, pick(a, e1, es)) < 0) {
			assign(pick(a, e2, es), pick(a, e1, es), es);
			assign(pick(a, e1, es), t, es);
		}
	}

	if (cmp(pick(a, e4, es), pick(a, e3, es))<0) {
		assign(t, pick(a, e4, es), es);
		assign(pick(a, e4, es), pick(a, e3, es), es);
		assign(pick(a, e3, es), t, es);
		if (cmp(t, pick(a, e2, es)) < 0) {
			assign(pick(a, e3, es), pick(a, e2, es), es);
			assign(pick(a, e2, es), t, es);
			if (cmp(t, pick(a, e1, es)) < 0) {
				assign(pick(a, e2, es), pick(a, e1, es), es);
				assign(pick(a, e1, es), t, es);
			}
		}
	}

	if (cmp(pick(a, e5, es), pick(a, e4, es))<0) {
		assign(t, pick(a, e5, es), es);
		assign(pick(a, e5, es), pick(a, e4, es), es);
		assign(pick(a, e4, es), t, es);
		if (cmp(t, pick(a, e3, es)) < 0) {
			assign(pick(a, e4, es), pick(a, e3, es), es);
			assign(pick(a, e3, es), t, es);
			if (cmp(t, pick(a, e2, es)) < 0) {
				assign(pick(a, e3, es), pick(a, e2, es), es);
				assign(pick(a, e2, es), t, es);
				if (cmp(t, pick(a, e1, es)) < 0) {
					assign(pick(a, e2, es), pick(a, e1, es), es);
					assign(pick(a, e1, es), t, es);

				}
			}
		}
	}

	// Pointers
	size_t less = left;  // The index of the first element of center part
	size_t great = right; // The index before the first element of right part

	if (cmp(pick(a, e1, es), pick(a, e2, es)) &&
		cmp(pick(a, e2, es), pick(a, e3, es)) &&
		cmp(pick(a, e3, es), pick(a, e4, es)) &&
		cmp(pick(a, e4, es), pick(a, e5, es))) {
		/*
		* Use the second and fourth of the five sorted elements as pivots.
		* These values are inexpensive approximations of the first and
		* second terciles of the array. Note that pivot1 <= pivot2.
		*/
		char pivot1[MAX_ES], pivot2[MAX_ES];
		assign(pivot1, pick(a, e2, es), es);
		assign(pivot2, pick(a, e4, es), es);

		/*
		* The first and the last elements to be sorted are moved to the
		* locations formerly occupied by the pivots. When partitioning
		* is complete, the pivots are swapped back into their final
		* positions, and excluded from subsequent sorting.
		*/
		assign(pick(a, e2, es), pick(a, left, es), es);
		assign(pick(a, e4, es), pick(a, right, es), es);

		/*
		* Skip elements, which are less or greater than pivot values.
		*/
		while (cmp(pick(a, ++less, es), pivot1) < 0);
		while (cmp(pick(a, --great, es), pivot2) > 0);

		/*
		* Partitioning:
		*
		*   left part           center part                   right part
		* +--------------------------------------------------------------+
		* |  < pivot1  |  pivot1 <= && <= pivot2  |    ?    |  > pivot2  |
		* +--------------------------------------------------------------+
		*               ^                          ^       ^
		*               |                          |       |
		*              less                        k     great
		*
		* Invariants:
		*
		*              all in (left, less)   < pivot1
		*    pivot1 <= all in [less, k)     <= pivot2
		*              all in (great, right) > pivot2
		*
		* Pointer k is the first index of ?-part.
		*/
		for (int k = less - 1; ++k <= great; ) {
			char ak[MAX_ES];
			assign(ak, pick(a, k, es), es);
			if (cmp(ak, pivot1) < 0) { // Move a[k] to left part
				assign(pick(a, k, es), pick(a, less, es), es);
				/*
				* Here and below we use "a[i] = b; i++;" instead
				* of "a[i++] = b;" due to performance issue.
				*/
				assign(pick(a, less, es), ak, es);
				++less;
			}
			else if (cmp(ak, pivot2) > 0) { // Move a[k] to right part
				while (cmp(pick(a, great, es), pivot2) > 0) {
					if (great-- == k) {
						goto jump1;
					}
				}
				if (cmp(pick(a, great, es), pivot1) < 0) { // a[great] <= pivot2
					assign(pick(a, k, es), pick(a, less, es), es);
					assign(pick(a, less, es), pick(a, great, es), es);
					++less;
				}
				else { // pivot1 <= a[great] <= pivot2
					assign(pick(a, k, es), pick(a, great, es), es);
				}
				/*
				* Here and below we use "a[i] = b; i--;" instead
				* of "a[i--] = b;" due to performance issue.
				*/
				assign(pick(a, great, es), ak, es);
				--great;
			}
		}
	jump1:
		// Swap pivots into their final positions
		assign(pick(a, left, es), pick(a, less - 1, es), es);
		assign(pick(a, less - 1, es), pivot1, es);
		assign(pick(a, right, es), pick(a, great + 1, es), es);
		assign(pick(a, great + 1, es), pivot2, es);

		// Sort left and right parts recursively, excluding known pivots
		dual_pivot_quick_sort_recursive(a, left, less - 2, es, cmp);
		dual_pivot_quick_sort_recursive(a, great + 2, right, es, cmp);

		/*
		* If center part is too large (comprises > 4/7 of the array),
		* swap internal pivot values to ends.
		*/
		if (less < e1 && e5 < great) {
			/*
			* Skip elements, which are equal to pivot values.
			*/
			while (!cmp(pick(a, less, es), pivot1)) {
				++less;
			}

			while (!cmp(pick(a, great, es), pivot2)) {
				--great;
			}

			/*
			* Partitioning:
			*
			*   left part         center part                  right part
			* +----------------------------------------------------------+
			* | == pivot1 |  pivot1 < && < pivot2  |    ?    | == pivot2 |
			* +----------------------------------------------------------+
			*              ^                        ^       ^
			*              |                        |       |
			*             less                      k     great
			*
			* Invariants:
			*
			*              all in (*,  less) == pivot1
			*     pivot1 < all in [less,  k)  < pivot2
			*              all in (great, *) == pivot2
			*
			* Pointer k is the first index of ?-part.
			*/
			for (int k = less - 1; ++k <= great; ) {
				char ak[MAX_ES];
				assign(ak, pick(a, k, es), es);
				if (!cmp(ak, pivot1)) { // Move a[k] to left part
					assign(pick(a, k, es), pick(a, less, es), es);
					assign(pick(a, less, es), ak, es);
					++less;
				}
				else if (!cmp(ak, pivot2)) { // Move a[k] to right part
					while (!cmp(pick(a, great, es), pivot2)) {
						if (great-- == k) {
							goto jump2;
						}
					}
					if (!cmp(pick(a, great, es), pivot1)) { // a[great] < pivot2
						assign(pick(a, k, es), pick(a, less, es), es);
						/*
						* Even though a[great] equals to pivot1, the
						* assignment a[less] = pivot1 may be incorrect,
						* if a[great] and pivot1 are floating-point zeros
						* of different signs. Therefore in float and
						* double sorting methods we have to use more
						* accurate assignment a[less] = a[great].
						*/
						assign(pick(a, less, es), pivot1, es);
						++less;
					}
					else { // pivot1 < a[great] < pivot2
						assign(pick(a, k, es), pick(a, great, es), es);
					}
					assign(pick(a, great, es), ak, es);
					--great;
				}
			}
		jump2:;
		}

		// Sort center part recursively
		dual_pivot_quick_sort_recursive(a, less, great, es, cmp);

	}
	else { // Partitioning with one pivot
		   /*
		   * Use the third of the five sorted elements as pivot.
		   * This value is inexpensive approximation of the median.
		   */
		char pivot[MAX_ES];
		assign(pivot, pick(a, e3, es), es);

		/*
		* Partitioning degenerates to the traditional 3-way
		* (or "Dutch National Flag") schema:
		*
		*   left part    center part              right part
		* +-------------------------------------------------+
		* |  < pivot  |   == pivot   |     ?    |  > pivot  |
		* +-------------------------------------------------+
		*              ^              ^        ^
		*              |              |        |
		*             less            k      great
		*
		* Invariants:
		*
		*   all in (left, less)   < pivot
		*   all in [less, k)     == pivot
		*   all in (great, right) > pivot
		*
		* Pointer k is the first index of ?-part.
		*/
		for (int k = less; k <= great; ++k) {
			if (!cmp(pick(a, k, es), pivot)) {
				continue;
			}
			char ak[MAX_ES];
			assign(ak, pick(a, k, es), es);
			if (cmp(ak, pivot) < 0) { // Move a[k] to left part
				assign(pick(a, k, es), pick(a, less, es), es);
				assign(pick(a, less, es), ak, es);
				++less;
			}
			else { // a[k] > pivot - Move a[k] to right part
				while (cmp(pick(a, great, es), pivot) > 0) {
					--great;
				}
				if (cmp(pick(a, great, es), pivot) < 0) { // a[great] <= pivot
					assign(pick(a, k, es), pick(a, less, es), es);
					assign(pick(a, less, es), pick(a, great, es), es);
					++less;
				}
				else { // a[great] == pivot
					   /*
					   * Even though a[great] equals to pivot, the
					   * assignment a[k] = pivot may be incorrect,
					   * if a[great] and pivot are floating-point
					   * zeros of different signs. Therefore in float
					   * and double sorting methods we have to use
					   * more accurate assignment a[k] = a[great].
					   */
					assign(pick(a, k, es), pivot, es);
				}
				assign(pick(a, great, es), ak, es);
				--great;
			}
		}

		/*
		* Sort left and right parts recursively.
		* All elements from center part are equal
		* and, therefore, already sorted.
		*/
		dual_pivot_quick_sort_recursive(a, left, less - 1, es, cmp);
		dual_pivot_quick_sort_recursive(a, great + 1, right, es, cmp);
	}
}

/* Dual-pivot quicksort implementation, based on JDK8 */
void dual_pivot_quick_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {

	dual_pivot_quick_sort_recursive(a, 0U, size - 1U, es, cmp);
}


/**
* TIM SORT
*/
static __inline void reverse_elements(void *dst, size_t start, size_t end,
	size_t es, int(*cmp) (const void *, const void *)) {
	while (1) {
		if (start >= end) {
			return;
		}

		swap(pick(dst, start, es), pick(dst, end, es), es);
		start++;
		end--;
	}
}

static size_t count_run(void *dst, const size_t start, const size_t size,
	size_t es, int(*cmp) (const void *, const void *)) {
	size_t curr;

	if (size - start == 1) {
		return 1;
	}

	if (start >= size - 2) {
		if (cmp(pick(dst, size - 2, es), pick(dst, size - 1, es)) > 0) {
			swap(pick(dst, size - 2, es), pick(dst, size - 1, es), es);
		}

		return 2;
	}

	curr = start + 2;

	if (cmp(pick(dst, start, es), pick(dst, start + 1, es)) <= 0) {
		/* increasing run */
		while (1) {
			if (curr == size - 1) {
				break;
			}

			if (cmp(pick(dst, curr - 1, es), pick(dst, curr, es)) > 0) {
				break;
			}

			curr++;
		}

		return curr - start;
	}
	else {
		/* decreasing run */
		while (1) {
			if (curr == size - 1) {
				break;
			}

			if (cmp(pick(dst, curr - 1, es), pick(dst, curr, es)) <= 0) {
				break;
			}

			curr++;
		}

		/* reverse in-place */
		reverse_elements(dst, start, curr - 1, es, cmp);
		return curr - start;
	}
}

static int check_invariant(TIM_SORT_RUN_T *stack, const int stack_curr) {
	size_t A, B, C;

	if (stack_curr < 2) {
		return 1;
	}

	if (stack_curr == 2) {
		const size_t A1 = stack[stack_curr - 2].length;
		const size_t B1 = stack[stack_curr - 1].length;

		if (A1 <= B1) {
			return 0;
		}

		return 1;
	}

	A = stack[stack_curr - 3].length;
	B = stack[stack_curr - 2].length;
	C = stack[stack_curr - 1].length;

	if ((A <= B + C) || (B <= C)) {
		return 0;
	}

	return 1;
}

static void tim_sort_resize(TEMP_STORAGE_T *store, const size_t new_size,
	size_t es, int(*cmp) (const void *, const void *)) {
	if (store->alloc < new_size) {
		char *tempstore = (char *)realloc(store->storage, new_size * es);

		if (tempstore == NULL) {
			fprintf(stderr, "Error allocating temporary storage for tim sort: need %lu bytes",
				(unsigned long)(es * new_size));
			exit(1);
		}

		store->storage = tempstore;
		store->alloc = new_size;
	}
}

static void tim_sort_merge(void *dst, const TIM_SORT_RUN_T *stack, const int stack_curr,
	TEMP_STORAGE_T *store, size_t es, int(*cmp) (const void *, const void *)) {
	const size_t A = stack[stack_curr - 2].length;
	const size_t B = stack[stack_curr - 1].length;
	const size_t curr = stack[stack_curr - 2].start;
	char *storage;
	size_t i, j, k;
	tim_sort_resize(store, min(A, B), es, cmp);
	storage = store->storage;

	/* left merge */
	if (A < B) {
		memcpy(storage, pick(dst, curr, es), A * es);
		i = 0;
		j = curr + A;

		for (k = curr; k < curr + A + B; k++) {
			if ((i < A) && (j < curr + A + B)) {
				if (cmp(pick(storage, i, es), pick(dst, j, es)) <= 0) {
					assign(pick(dst, k, es), pick(storage, i++, es), es);
				}
				else {
					assign(pick(dst, k, es), pick(dst, j++, es), es);
				}
			}
			else if (i < A) {
				assign(pick(dst, k, es), pick(storage, i++, es), es);
			}
			else {
				break;
			}
		}
	}
	else {
		/* right merge */
		memcpy(storage, pick(dst, curr + A, es), B * es);
		i = B;
		j = curr + A;
		k = curr + A + B;

		while (k-- > curr) {
			if ((i > 0) && (j > curr)) {
				if (cmp(pick(dst, j - 1, es), pick(storage, i - 1, es)) > 0) {
					assign(pick(dst, k, es), pick(dst, --j, es), es);
				}
				else {
					assign(pick(dst, k, es), pick(storage, --i, es), es);
				}
			}
			else if (i > 0) {
				assign(pick(dst, k, es), pick(storage, --i, es), es);
			}
			else {
				break;
			}
		}
	}
}

static int tim_sort_collapse(void *dst, TIM_SORT_RUN_T *stack, int stack_curr,
	TEMP_STORAGE_T *store, const size_t size, size_t es, int(*cmp) (const void *, const void *)) {
	while (1) {
		size_t A, B, C, D;
		int ABC, BCD, CD;

		/* if the stack only has one thing on it, we are done with the collapse */
		if (stack_curr <= 1) {
			break;
		}

		/* if this is the last merge, just do it */
		if ((stack_curr == 2) && (stack[0].length + stack[1].length == size)) {
			tim_sort_merge(dst, stack, stack_curr, store, es, cmp);
			stack[0].length += stack[1].length;
			stack_curr--;
			break;
		}
		/* check if the invariant is off for a stack of 2 elements */
		else if ((stack_curr == 2) && (stack[0].length <= stack[1].length)) {
			tim_sort_merge(dst, stack, stack_curr, store, es, cmp);
			stack[0].length += stack[1].length;
			stack_curr--;
			break;
		}
		else if (stack_curr == 2) {
			break;
		}

		B = stack[stack_curr - 3].length;
		C = stack[stack_curr - 2].length;
		D = stack[stack_curr - 1].length;

		if (stack_curr >= 4) {
			A = stack[stack_curr - 4].length;
			ABC = (A <= B + C);
		}
		else {
			ABC = 0;
		}

		BCD = (B <= C + D) || ABC;
		CD = (C <= D);

		/* Both invariants are good */
		if (!BCD && !CD) {
			break;
		}

		/* left merge */
		if (BCD && !CD) {
			tim_sort_merge(dst, stack, stack_curr - 1, store, es, cmp);
			stack[stack_curr - 3].length += stack[stack_curr - 2].length;
			stack[stack_curr - 2] = stack[stack_curr - 1];
			stack_curr--;
		}
		else {
			/* right merge */
			tim_sort_merge(dst, stack, stack_curr, store, es, cmp);
			stack[stack_curr - 2].length += stack[stack_curr - 1].length;
			stack_curr--;
		}
	}

	return stack_curr;
}

static __inline int push_next(void *a,
	const size_t size,
	TEMP_STORAGE_T *store,
	const size_t minrun,
	TIM_SORT_RUN_T *run_stack,
	size_t *stack_curr,
	size_t *curr,
	const size_t es, int(*cmp) (const void *, const void *)) {
	size_t len = count_run(a, *curr, size, es, cmp);
	size_t run = minrun;

	if (run > size - *curr) {
		run = size - *curr;
	}

	if (run > len) {
		binary_insertion_sort_start(pick(a, *curr, es), len, run, es, cmp);
		len = run;
	}

	run_stack[*stack_curr].start = *curr;
	run_stack[*stack_curr].length = len;
	(*stack_curr)++;
	*curr += len;

	if (*curr == size) {
		/* finish up */
		while (*stack_curr > 1) {
			tim_sort_merge(a, run_stack, *stack_curr, store, es, cmp);
			run_stack[*stack_curr - 2].length += run_stack[*stack_curr - 1].length;
			(*stack_curr)--;
		}

		if (store->storage != NULL) {
			free(store->storage);
			store->storage = NULL;
		}

		return 0;
	}

	return 1;
}

static __inline int compute_minrun(const uint64_t size) {
	const int top_bit = 64 - CLZ(size);
	const int shift = max(top_bit, 6) - 6;
	const int minrun = size >> shift;
	const uint64_t mask = (1ULL << shift) - 1;

	if (mask & size) {
		return minrun + 1;
	}

	return minrun;
}

/* implementation of tim sort */
void tim_sort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	size_t minrun;
	TEMP_STORAGE_T _store, *store;
	TIM_SORT_RUN_T run_stack[TIM_SORT_STACK_SIZE];
	size_t stack_curr = 0;
	size_t curr = 0;

	/* don't bother sorting an array of size 1 */
	if (size <= 1) {
		return;
	}

	if (size < 64) {
		binary_insertion_sort(a, size, es, cmp);
		return;
	}

	/* compute the minimum run length */
	minrun = compute_minrun(size);
	/* temporary storage for merges */
	store = &_store;
	store->alloc = 0;
	store->storage = NULL;

	if (!push_next(a, size, store, minrun, run_stack, &stack_curr, &curr, es, cmp)) {
		return;
	}

	if (!push_next(a, size, store, minrun, run_stack, &stack_curr, &curr, es, cmp)) {
		return;
	}

	if (!push_next(a, size, store, minrun, run_stack, &stack_curr, &curr, es, cmp)) {
		return;
	}

	while (1) {
		if (!check_invariant(run_stack, stack_curr)) {
			stack_curr = tim_sort_collapse(a, run_stack, stack_curr, store, size, es, cmp);
			continue;
		}

		if (!push_next(a, size, store, minrun, run_stack, &stack_curr, &curr, es, cmp)) {
			return;
		}
	}
}

/*
* radix sort implementation, based on https://www.geeksforgeeks.org/radix-sort/
* this sorting algorithm only works on integer arrays
*/
void radix_sort(int *dst, const size_t size) {
	if (!size) {
		return;
	}
	//find the maximal number to know number of digits
	int m = dst[0];
	for (size_t i = 1; i < size; i++) {
		m = max(m, dst[i]);
	}

	int* output = malloc(sizeof(int)*size);
	int count[RADIX_SORT_BASE];
	for (int exp = 1; m / exp > 0; exp *= RADIX_SORT_BASE) {
		memset(count, 0, RADIX_SORT_BASE * sizeof(int));
		// store count of occurrences in count[]
		for (size_t i = 0; i < size; i++) {
			count[(dst[i] / exp) % RADIX_SORT_BASE]++;
		}
		// change count[i] so that count[i] now contains actual
		// position of this digit in output[]
		for (size_t i = 1; i < RADIX_SORT_BASE; i++) {
			count[i] += count[i - 1];
		}

		// build the output array
		for (int i = size - 1; i >= 0; i--) {
			output[count[(dst[i] / exp) % RADIX_SORT_BASE] - 1] = dst[i];
			count[(dst[i] / exp) % RADIX_SORT_BASE]--;
		}

		// copy the output array to arr[], so that arr[] now
		// contains sorted numbers according to current digit
		memcpy(dst, output, size * sizeof(int));
	}

	free(output);
}

static char *med3(char *a, char *b, char *c,
	int(*cmp) (const void *, const void *));
static void swapfunc(char *, char *, size_t, int);

/*
* Qsort routine based on J. L. Bentley and M. D. McIlroy,
* "Engineering a sort function",
* Software--Practice and Experience 23 (1993) 1249-1265.
*
* We have modified their original by adding a check for already-sorted input,
* which seems to be a win per discussions on pgsql-hackers around 2006-03-21.
*
* Also, we recurse on the smaller partition and iterate on the larger one,
* which ensures we cannot recurse more than log(N) levels (since the
* partition recursed to is surely no more than half of the input).  Bentley
* and McIlroy explicitly rejected doing this on the grounds that it's "not
* worth the effort", but we have seen crashes in the field due to stack
* overrun, so that judgment seems wrong.
*/

#define swapcode(TYPE, parmi, parmj, n) \
do {		\
	size_t i = (n) / sizeof (TYPE);			\
	TYPE *pi = (TYPE *)(void *)(parmi);			\
	TYPE *pj = (TYPE *)(void *)(parmj);			\
	do {						\
		TYPE	t = *pi;			\
		*pi++ = *pj;				\
		*pj++ = t;				\
		} while (--i > 0);				\
} while (0)

#define SWAPINIT(a, es) swaptype = ((char *)(a) - (char *)0) % sizeof(long) || \
	(es) % sizeof(long) ? 2 : (es) == sizeof(long)? 0 : 1;

static void
swapfunc(char *a, char *b, size_t n, int swaptype)
{
	if (swaptype <= 1)
		swapcode(long, a, b, n);
	else
		swapcode(char, a, b, n);
}

#define swap(a, b)						\
	if (swaptype == 0) {					\
		long t = *(long *)(void *)(a);			\
		*(long *)(void *)(a) = *(long *)(void *)(b);	\
		*(long *)(void *)(b) = t;			\
	} else							\
		swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) if ((n) > 0) swapfunc(a, b, n, swaptype)

static char *
med3(char *a, char *b, char *c, int(*cmp) (const void *, const void *))
{
	return cmp(a, b) < 0 ?
		(cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
		: (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}

/* heap sort: based on wikipedia */

static inline void
heap_shift_down(void *a, const size_t start, const size_t end,
	int swaptype, const size_t es, int(*cmp) (const void *, const void *)) {
	size_t root = start;

	while ((root << 1) + 1 <= end) {
		size_t child = (root << 1) + 1;

		if ((child < end) && cmp((char*)a + child * es, (char*)a + (child + 1)*es) < 0) {
			child++;
		}

		if (cmp((char*)a + root * es, (char*)a + child * es) < 0) {
			swap((char*)a + root * es, (char*)a + child * es);
			root = child;
		}
		else {
			return;
		}
	}
}

static inline void
heap_sort(void *a, const size_t size, int swaptype, const size_t es, int(*cmp) (const void *, const void *)) {

	size_t start, end;
	/* don't bother sorting an array of size <= 1 */
	if (size <= 1) {
		return;
	}

	end = size - 1;

	/* heapify */
	start = (end - 1) >> 1;
	while (1) {
		heap_shift_down(a, start, end, swaptype, es, cmp);

		if (start == 0) {
			break;
		}

		start--;
	}

	while (end > 0) {
		swap((char*)a + end * es, (char*)a);
		heap_shift_down(a, 0, end - 1, swaptype, es, cmp);
		end--;
	}
}

static void
pg_qsort_recursive(void *a, size_t n, size_t depth, int swaptype, size_t es, int(*cmp) (const void *, const void *))
{
	char	   *pa,
		*pb,
		*pc,
		*pd,
		*pl,
		*pm,
		*pn;
	size_t		d1,
		d2;
	int			r,
		presorted;

loop:
	if (n < 7)
	{
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl >(char *) a && cmp(pl - es, pl) > 0;
				pl -= es)
				swap(pl, pl - es);
		return;
	}
	presorted = 1;
	for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
	{
		if (cmp(pm - es, pm) > 0)
		{
			presorted = 0;
			break;
		}
	}
	if (presorted)
		return;
	// convert to heap sort if exceed depth limit
	if (!depth) {
		heap_sort(a, n, swaptype, es, cmp);
		return;
	}

	pm = (char *)a + (n / 2) * es;
	if (n > 7)
	{
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40)
		{
			size_t		d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp);
			pm = med3(pm - d, pm, pm + d, cmp);
			pn = med3(pn - 2 * d, pn - d, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp);
	}
	swap(a, pm);
	pa = pb = (char *)a + es;
	pc = pd = (char *)a + (n - 1) * es;
	for (;;)
	{
		while (pb <= pc && (r = cmp(pb, a)) <= 0)
		{
			if (r == 0)
			{
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0)
		{
			if (r == 0)
			{
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		pb += es;
		pc -= es;
	}
	pn = (char *)a + n * es;
	d1 = Min(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	d1 = Min(pd - pc, pn - pd - es);
	vecswap(pb, pn - d1, d1);
	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2)
	{
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es)
			pg_qsort_recursive(a, d1 / es, depth - 1, swaptype, es, cmp);
		if (d2 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* pg_qsort_recursive(pn - d2, d2 / es, depth - 1, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			depth--;
			goto loop;
		}
	}
	else
	{
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es)
			pg_qsort_recursive(pn - d2, d2 / es, depth - 1, swaptype, es, cmp);
		if (d1 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* pg_qsort_recursive(a, d1 / es, depth - 1, es, cmp); */
			n = d1 / es;
			depth--;
			goto loop;
		}
	}
}


void
pg_qsort(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	int swaptype, presorted = 1;
	char* pm, pl;
	SWAPINIT(a, es);
	pg_qsort_recursive(a, size, 2 * log(size), swaptype, es, cmp);
};


static void
pg_qsort_once_recursive(void *a, size_t n, size_t depth, int swaptype, size_t es, int(*cmp) (const void *, const void *))
{
	char	   *pa,
		*pb,
		*pc,
		*pd,
		*pl,
		*pm,
		*pn;
	size_t		d1,
		d2;
	int			r,
		presorted;

loop:
	if (n < 7)
	{
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl >(char *) a && cmp(pl - es, pl) > 0;
				pl -= es)
				swap(pl, pl - es);
		return;
	}
	// convert to heap sort if exceed depth limit
	if (!depth) {
		heap_sort(a, n, swaptype, es, cmp);
		return;
	}

	pm = (char *)a + (n / 2) * es;
	if (n > 7)
	{
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40)
		{
			size_t		d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp);
			pm = med3(pm - d, pm, pm + d, cmp);
			pn = med3(pn - 2 * d, pn - d, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp);
	}
	swap(a, pm);
	pa = pb = (char *)a + es;
	pc = pd = (char *)a + (n - 1) * es;
	for (;;)
	{
		while (pb <= pc && (r = cmp(pb, a)) <= 0)
		{
			if (r == 0)
			{
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0)
		{
			if (r == 0)
			{
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		pb += es;
		pc -= es;
	}
	pn = (char *)a + n * es;
	d1 = Min(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	d1 = Min(pd - pc, pn - pd - es);
	vecswap(pb, pn - d1, d1);
	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2)
	{
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es)
			pg_qsort_once_recursive(a, d1 / es, depth - 1, swaptype, es, cmp);
		if (d2 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* pg_qsort_once_recursive(pn - d2, d2 / es, depth - 1, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			depth--;
			goto loop;
		}
	}
	else
	{
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es)
			pg_qsort_once_recursive(pn - d2, d2 / es, depth - 1, swaptype, es, cmp);
		if (d1 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* pg_qsort_once_recursive(a, d1 / es, depth - 1, es, cmp); */
			n = d1 / es;
			depth--;
			goto loop;
		}
	}
}


void
pg_qsort_once(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	int swaptype, presorted = 1;
	char* pm, pl;
	SWAPINIT(a, es);
	if (size < 7)
	{
		for (pm = (char *)a + es; pm < (char *)a + size * es; pm += es)
			for (pl = pm; pl >(char *) a && cmp(pl - es, pl) > 0;
				pl -= es)
				swap(pl, pl - es);
		return;
	}
	presorted = 1;
	for (pm = (char *)a + es; pm < (char *)a + size * es; pm += es)
	{
		if (cmp(pm - es, pm) > 0)
		{
			presorted = 0;
			break;
		}
	}
	if (presorted)
		return;
	pg_qsort_once_recursive(a, size, 2 * log(size), swaptype, es, cmp);
};
/*
* qsort comparator wrapper for strcmp.
*/
int
pg_qsort_strcmp(const void *a, const void *b)
{
	return strcmp(*(const char *const *)a, *(const char *const *)b);
}


void
old_pg_qsort(void *a, size_t n, size_t es, int(*cmp) (const void *, const void *))
{
	char	   *pa,
		*pb,
		*pc,
		*pd,
		*pl,
		*pm,
		*pn;
	size_t		d1,
		d2;
	int			r,
		swaptype,
		presorted;

loop:SWAPINIT(a, es);
	if (n < 7)
	{
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl >(char *) a && cmp(pl - es, pl) > 0;
				pl -= es)
				swap(pl, pl - es);
		return;
	}
	presorted = 1;
	for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
	{
		if (cmp(pm - es, pm) > 0)
		{
			presorted = 0;
			break;
		}
	}
	if (presorted)
		return;
	pm = (char *)a + (n / 2) * es;
	if (n > 7)
	{
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40)
		{
			size_t		d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp);
			pm = med3(pm - d, pm, pm + d, cmp);
			pn = med3(pn - 2 * d, pn - d, pn, cmp);
		}
		pm = med3(pl, pm, pn, cmp);
	}
	swap(a, pm);
	pa = pb = (char *)a + es;
	pc = pd = (char *)a + (n - 1) * es;
	for (;;)
	{
		while (pb <= pc && (r = cmp(pb, a)) <= 0)
		{
			if (r == 0)
			{
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0)
		{
			if (r == 0)
			{
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		pb += es;
		pc -= es;
	}
	pn = (char *)a + n * es;
	d1 = Min(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	d1 = Min(pd - pc, pn - pd - es);
	vecswap(pb, pn - d1, d1);
	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2)
	{
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es)
			old_pg_qsort(a, d1 / es, es, cmp);
		if (d2 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* old_pg_qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	}
	else
	{
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es)
			old_pg_qsort(pn - d2, d2 / es, es, cmp);
		if (d1 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* old_pg_qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

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

void
rand_pg_qsort(void *a, size_t n, size_t es, int(*cmp) (const void *, const void *))
{
	char	   *pa,
		*pb,
		*pc,
		*pd,
		*pl,
		*pm,
		*pn;
	size_t		d1,
		d2;
	int			r,
		swaptype,
		presorted;

loop:SWAPINIT(a, es);
	if (n < 7)
	{
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; pl >(char *) a && cmp(pl - es, pl) > 0;
				pl -= es)
				swap(pl, pl - es);
		return;
	}
	presorted = 1;
	for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
	{
		if (cmp(pm - es, pm) > 0)
		{
			presorted = 0;
			break;
		}
	}
	if (presorted)
		return;
	pm = (char*)a + random_int(n-1)*es;
	swap(a, pm);
	pa = pb = (char *)a + es;
	pc = pd = (char *)a + (n - 1) * es;
	for (;;)
	{
		while (pb <= pc && (r = cmp(pb, a)) <= 0)
		{
			if (r == 0)
			{
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0)
		{
			if (r == 0)
			{
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		pb += es;
		pc -= es;
	}
	pn = (char *)a + n * es;
	d1 = Min(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	d1 = Min(pd - pc, pn - pd - es);
	vecswap(pb, pn - d1, d1);
	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2)
	{
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es)
			rand_pg_qsort(a, d1 / es, es, cmp);
		if (d2 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* rand_pg_qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	}
	else
	{
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es)
			rand_pg_qsort(pn - d2, d2 / es, es, cmp);
		if (d1 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* rand_pg_qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}


void heap_sort_wrapper(void *a, const size_t size, const size_t es, int(*cmp) (const void *, const void *)) {
	int swaptype;
	SWAPINIT(a, es);
	heap_sort(a, size, swaptype, es, cmp);
}