#ifndef RELAXATION_H
#define RELAXATION_H

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    size_t size;
    double *values;
    double *prev_values;
    double precision;
    size_t start_row;
    size_t end_row;
    bool *converged;
} solve_args;

void run(size_t size, double (*values)[size][size]);

void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision);
void solve_chunk(solve_args *args);

void array_2d_alloc_no_err(size_t rows, size_t cols, double (**values)[rows][cols]);
void array_2d_print(size_t rows, size_t cols, double (*array)[rows][cols], FILE *stream);
void thread_handlers_alloc_no_err(size_t thread_count, pthread_t *handles, solve_args (**args)[thread_count], bool *converged);

// void panic(const char *error_str);
// void panic_malloc();

#endif