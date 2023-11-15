/*
To build, use GCC with the -lpthread flag:
cc -lpthread -o relaxation relaxation.c
*/

#ifndef RELAXATION_H
#define RELAXATION_H

#include <stdio.h>
#include <stdlib.h>

typedef struct solve_args solve_args;

/*
Contains all arguments for `solve_chunk` so that it can be executed on a thread.
`matrix` and `prev_matrix` point to 2D square arrays.
This is despite being single pointers.
They are like this because structs fields cannot use standard array syntax.
*/
struct solve_args
{
    size_t size;         // The length of one side of `matrix`.
    double *matrix;      // Points to a 2D square array of length `size`.
    double *prev_matrix; // Points to a 2D square array of length `size`.
    size_t start_row;    // Row of `matrix` that computation starts.
    size_t end_row;      // Row of `matrix` that computation finishes before.
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

/*
A helper function to allocate memory for all given arguments.
It guarantees allocations either all succeed or all fail.
Memory is not leaked if allocaitons fail.

`size` is the length of one side of `matrix`.
`matrix` will be allocated a pointer for a square 2D array of length `size`.
`thread_count` controls the number of elements in `handles` & `args`.
`handles` will be allocated a pointer for an array of size `thread_count`.
`args` will be allocated a pointer for an array of size `thread_count`.

Returns 0 if exited normally, 1 if allocation fails.
*/
int solve_try_alloc(
    size_t size,
    double (**matrix)[size][size],
    size_t thread_count,
    pthread_t **handles,
    solve_args **args);

/*
A helper function to allocate memory for a 2D square matrix.
It guarantees that memory is allocated as one contiguous block, allowing
stdlib functions such as memcmp & memcpy can be used with the resulting array.

`size` is the length of one side of `matrix`.
`matrix` will be allocated a pointer for a square 2D array of length `size`.
*/
int array_2d_try_alloc(
    size_t size,
    double (**matrix)[size][size]);

/*
A helper function to print a square 2D array to a file or output stream.

`size` is the length of one side of `matrix`.
`matrix` points to a square 2D array of length `size`
`stream` points to a file or output stream.
*/
void array_2d_print(
    size_t size,
    double (*matrix)[size][size],
    FILE *stream);

#endif