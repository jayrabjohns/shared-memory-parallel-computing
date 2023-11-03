#ifndef RELAXATION_H
#define RELAXATION_H

#include <stdio.h>
#include <stdlib.h>

typedef struct solve_args solve_args;
struct solve_args
{
    size_t size;
    double *values;
    double *prev_values;
    double precision;
    size_t start_row;
    size_t end_row;
    bool *converged;
};

void run(size_t size, double (*values)[size][size]);

void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision);
void *solve_chunk(solve_args *args);

void array_2d_alloc_no_err(size_t rows, size_t cols, double (**values)[rows][cols]);
void array_2d_print(size_t rows, size_t cols, double (*array)[rows][cols], FILE *stream);
bool try_alloc_thread_handlers(size_t thread_count, pthread_t **handles, solve_args **args);

// void panic(const char *error_str);
// void panic_malloc();

#endif