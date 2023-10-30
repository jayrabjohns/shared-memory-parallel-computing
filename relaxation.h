#ifndef RELAXATION_H
#define RELAXATION_H

#include <stdio.h>
#include <stdlib.h>

void run(size_t size, double (*values)[size][size]);

void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision);

void array_2d_alloc_no_err(size_t rows, size_t cols, double (**values)[rows][cols]);
void array_2d_print(size_t rows, size_t cols, double array[rows][cols]);

// void panic(const char *error_str);
// void panic_malloc();

#endif