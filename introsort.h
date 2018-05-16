#pragma once
/*
*	introsort.h: introsort algorithm
*
*	Modfied from vanilla QuickSort code. When quicksort recursive depth is out of bound,
*	turn to use heap sort
*/
#include <math.h>
#include <memory.h>
#include "swap.h"

#define Min(x, y)		((x) < (y) ? (x) : (y))

/* To heapify a subtree rooted with node i which is an index in arr[]. n is size of heap */
void heapify(char* begin, int n, int i, int es, int swaptype, int(*cmp) (const void *, const void *))
{
	int largest = i;  // Initialize largest as root
	int l = 2 * i + 1;  // left = 2*i + 1
	int r = 2 * i + 2;  // right = 2*i + 2

	// If left child is larger than root
	if (l < n && cmp(begin + l * es, begin + largest * es)>0)
		largest = l;

	// If right child is larger than largest so far
	if (r < n && cmp(begin + r * es, begin + largest * es)>0)
		largest = r;

	// If largest is not root
	if (largest != i)
	{
		swap(begin+i*es, begin + largest * es);

		// Recursively heapify the affected sub-tree
		heapify(begin, n, largest, es, swaptype, cmp);
	}
}

/* Heap sort subroutine */
void heap_sort(char* a, int n, int es, int swaptype, int(*cmp) (const void *, const void *))
{
	// Build heap (rearrange array)
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(a, n, i, es, swaptype, cmp);

	// One by one extract an element from heap
	for (int i = n - 1; i >= 0; i--)
	{
		// Move current root to end
		swap(a, a + i * es);

		// call max heapify on the reduced heap
		heapify(a, i, 0, es, swaptype, cmp);
	}
}

static char *med3(char *a, char *b, char *c, int(*cmp) (const void *, const void *))
{
	return cmp(a, b) < 0 ?
		(cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
		: (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}

/* Introsort recursive funtion */
void
introsort_util(void *a, size_t n, size_t es, int depth_limit, int(*cmp) (const void *, const void *))
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
	/* depth out of bound */
	if (!depth_limit)
	{
		//Use heap sort
		heap_sort((char*)a, n, es, swaptype, cmp);
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
	swap((char*)a, pm);
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
	vecswap((char*)a, pb - d1, d1);
	d1 = Min(pd - pc, pn - pd - es);
	vecswap(pb, pn - d1, d1);
	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2)
	{
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es)
			introsort_util(a, d1 / es, es, depth_limit - 1, cmp);
		if (d2 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* pg_qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	}
	else
	{
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es)
			introsort_util(pn - d2, d2 / es, es, depth_limit - 1, cmp);
		if (d1 > es)
		{
			/* Iterate rather than recurse to save stack space */
			/* pg_qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

/* Implementation of introsort*/
void introsort(void *a, size_t n, size_t es, int(*cmp) (const void *, const void *))
{
	int depth_limit = 1.5 * log(n);
	// Perform a recursive introsort
	introsort_util((char*)a, n, es, depth_limit, cmp);

	return;
}