# PostgreSQLSorting
Sorting routines and benchmark for PostgreSQL

### sort.h
Benchmark implementation of sorting algorithms

Based on Swenson's [sort.h](https://github.com/swenson/sort/blob/master/sort.h)

**Modifications**: add ```intro_sort```, ```dual_pivot_quick_sort``` and ```radix_sort```

### test.c
Used to test sorting routines in ```sort.h```

Currently supported data type: ```int```, ```char```, self-defined ```string``` structure

Able to count the number of comparisons
