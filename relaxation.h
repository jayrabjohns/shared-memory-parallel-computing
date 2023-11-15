/*
To build, use GCC with the -lpthread flag:
cc -lpthread -o relaxation relaxation.c
*/

#ifndef RELAXATION_H
#define RELAXATION_H

#include <stdio.h>
#include <stdlib.h>

typedef struct solve_args solve_args;
struct solve_args
{
    size_t size;
    double *matrix;
    double *prev_matrix;
    size_t start_row;
    size_t end_row;
};

/*
Performs the relaxation technique on a given matrix in parallel.

`size` is the length of one side of the matrix.
`matrix` points to a 2D square matrix with length equal to `size`.
`thread_count` is the number of threads to use.
`precision` is the max difference between iterations to be classed as converged.
*/
int solve(
    size_t size,
    double (*matrix)[size][size],
    size_t thread_count,
    double precision);

/*
Performs the relaxation technique on a subset of rows for a given matrix.

It has its arguments packaged as a struct so it is able to be passed as a
function pointer when creating a pthread. It returns void* for the same reason.
*/
void *solve_chunk(solve_args *args);

/*
Performs the relaxation technique on a given matrix synchronously.

`size` is the length of one side of the matrix.
`matrix` points to a 2D square matrix with length equal to `size`.
`precision` is the max difference between iterations to be classed as converged.
*/
int solve_sync(
    size_t size,
    double (*matrix)[size][size],
    double precision);

/*
Checks if `m1` has converged with resepect to `m2`.
In this case, all elements of `m1` should differ by at most `precision`
from respective elements in `m2`.

`precision` is the max difference between iterations to be classed as converged.
`size` is the length of one side of `m1` or `m2`.
`m1` points to a 2D square matrix with length equal to `size`.
`m2` points to a 2D square matrix with length equal to `size`.
*/
bool matrix_has_converged(
    double precision,
    size_t size,
    const double (*m1)[size][size],
    const double (*m2)[size][size]);

int solve_try_alloc(
    size_t size,
    double (**matrix)[size][size],
    size_t thread_count,
    pthread_t **handles,
    solve_args **args);

int array_2d_try_alloc(
    size_t size,
    double (**matrix)[size][size]);

void array_2d_print(
    size_t size,
    double (*array)[size][size],
    FILE *stream);

#endif