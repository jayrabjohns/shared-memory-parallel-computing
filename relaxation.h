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
    size_t start_row;
    size_t end_row;
};

int solve(
    size_t size, double (*values)[size][size],
    size_t thread_count, double precision);

void *solve_chunk(solve_args *args);

int solve_sync(size_t size, double (*values)[size][size], double precision);

bool matrix_has_converged(
    double precision, size_t size,
    const double (*m1)[size][size], const double (*m2)[size][size]);

int solve_try_alloc(
    size_t size, double (**matrix)[size][size],
    size_t thread_count, pthread_t **handles, solve_args **args);

int array_2d_try_alloc(size_t size, double (**values)[size][size]);

void array_2d_print(size_t size, double (*array)[size][size], FILE *stream);

#endif