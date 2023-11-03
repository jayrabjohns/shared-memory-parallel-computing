#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <limits.h>

#include "relaxation.h"

const size_t MAX_ITERATIONS = ULONG_MAX;

int main(void)
{
    static const size_t size = 10000;
    const double precision = 0.01;
    const size_t thread_count = 8;

    double(*buf)[size][size];
    array_2d_alloc_no_err(size, size, &buf);

    for (size_t j = 0; j < size; j++)
        (*buf)[0][j] = 1.0f;

    for (size_t i = 1; i < size; i++)
    {
        (*buf)[i][0] = 1.0f;
        for (size_t j = 1; j < size; j++)
            (*buf)[i][j] = 0.0f;
    }

    solve(size, buf, thread_count, precision);

    double(*solve_sync_buf)[size][size];
    array_2d_alloc_no_err(size, size, &solve_sync_buf);

    for (size_t j = 0; j < size; j++)
        (*solve_sync_buf)[0][j] = 1.0f;

    for (size_t i = 1; i < size; i++)
    {
        (*solve_sync_buf)[i][0] = 1.0f;
        for (size_t j = 1; j < size; j++)
            (*solve_sync_buf)[i][j] = 0.0f;
    }

    solve_sync(size, solve_sync_buf, precision);

    int rc = memcmp(buf, solve_sync_buf, sizeof(*buf));
    if (rc == 0)
    {
        printf("PASS solution matches synchronous implementation\n");
    }
    else
    {
        printf("FAIL solution doesn't match synchronous implementation\n");
        printf("\nsync impl result\n");
        array_2d_print(size, size, solve_sync_buf, stdout);
        printf("async impl result:\n");
        array_2d_print(size, size, buf, stdout);
    }

    free(solve_sync_buf);
    free(buf);
    return 0;
}

void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision)
{
    thread_count = (size < thread_count ? size : thread_count);
    const size_t rows_per_thread = (size - 2) / thread_count;
    // With this way of assigning rows per thread, there's an edge case where (thread_count - 2) > size and all work get delegates the final thread (since the division gets rounded to 0)
    // This should be fine, because in any reasonable case where this would be noticeable, the size of the problem will be far higher than the number of cores available (asusming one thread per core)

    double(*prev_values)[size][size];
    array_2d_alloc_no_err(size, size, &prev_values);
    memcpy(prev_values, values, sizeof(*prev_values));

    // bool *converged = malloc(thread_count * sizeof(bool));
    // pthread_t *handles = malloc(thread_count * sizeof(pthread_t));
    // solve_args(*args) = malloc(sizeof(solve_args));
    // bool *thread_converged;
    pthread_t *handles;
    solve_args *args;
    if (!try_alloc_thread_handlers(thread_count, &handles, &args))
        return; // false

    for (size_t i = 0; i < thread_count; i++)
    {
        args[i].size = size;
        args[i].values = (double *)values;
        args[i].prev_values = (double *)prev_values;
        args[i].precision = precision;
        args[i].start_row = i * rows_per_thread + 1;
        args[i].end_row = (i == thread_count - 1 ? (size - 1) : (args[i].start_row + rows_per_thread));
        args[i].converged = true;

        printf("thread: %ld; start_row: %ld; end_row: %ld\n", i, args[i].start_row, args[i].end_row);
    }

    size_t iterations = 0;
    bool converged = false;
    while (!converged && iterations < MAX_ITERATIONS)
    {
        for (size_t i = 0; i < thread_count; i++)
        {
            args[i].converged = true;
            args[i].values = (double *)values;
            args[i].prev_values = (double *)prev_values;
            pthread_create(&handles[i], NULL, (void *(*)(void *))solve_chunk, &args[i]);
        }

        for (size_t i = 0; i < thread_count; i++)
        {
            pthread_join(handles[i], NULL);
        }

        converged = true;
        for (size_t row = 0; row < size && converged; row++)
        {
            for (size_t col = 0; col < size && converged; col++)
            {
                const double diff = fabs((*values)[row][col] - (*prev_values)[row][col]);
                if (diff > precision)
                    converged = false;
            }
        }

        ++iterations;
        // printf("iteration: %ld\n", iterations);
        // array_2d_print(size, size, values, stdout);
        memcpy(prev_values, values, sizeof(*prev_values));
    }

    printf("[ASYNC] solved in %ld iterations\n", iterations);

    free(prev_values);
    free(handles);
    free(args);
}

void *solve_chunk(solve_args *args)
{
    double(*values)[args->size][args->size] = (double(*)[args->size][args->size])args->values;
    double(*prev_values)[args->size][args->size] = (double(*)[args->size][args->size])args->prev_values;

    for (size_t row = args->start_row; row < args->end_row; row++)
    {
        for (size_t col = 1; col < args->size - 1; col++)
        {
            const double neighbours_sum = (*prev_values)[row - 1][col] + (*prev_values)[row + 1][col] + (*prev_values)[row][col - 1] + (*prev_values)[row][col + 1];
            (*values)[row][col] = neighbours_sum / 4.0;

            const double diff = fabs((*values)[row][col] - (*prev_values)[row][col]);
            if (diff > args->precision)
                args->converged = false;
        }
    }

    return NULL;
}

void solve_sync(size_t size, double (*values)[size][size], double precision)
{
    double(*prev_values)[size][size];
    array_2d_alloc_no_err(size, size, &prev_values);
    memcpy(prev_values, values, sizeof(*prev_values));

    size_t iterations = 0;
    bool converged = false;
    while (!converged && iterations < MAX_ITERATIONS)
    {
        converged = true;
        for (size_t row = 1; row < size - 1; row++)
        {
            for (size_t col = 1; col < size - 1; col++)
            {
                const double neighbours_sum = (*prev_values)[row - 1][col] + (*prev_values)[row + 1][col] + (*prev_values)[row][col - 1] + (*prev_values)[row][col + 1];
                (*values)[row][col] = neighbours_sum / 4.0;

                const double diff = fabs((*values)[row][col] - (*prev_values)[row][col]);
                if (diff > precision)
                    converged = false;
            }
        }

        ++iterations;
        // printf("iteration: %ld\n", iterations);
        // array_2d_print(size, size, values, stdout);
        memcpy(prev_values, values, sizeof(*prev_values));
    }

    printf("[SYNC] solved in %ld iterations\n", iterations);

    free(prev_values);
}

void array_2d_alloc_no_err(size_t rows, size_t cols, double (**values)[rows][cols])
{
    *values = malloc(rows * cols * sizeof(double));
    if (*values == NULL)
    {
        fprintf(stderr, "Cannot allocate memory for %ldx%ld array.\n", rows, cols);
        exit(1);
    }
}

bool try_alloc_thread_handlers(size_t thread_count, pthread_t **handles, solve_args **args)
{
    *handles = malloc(thread_count * sizeof(pthread_t));
    if (*handles == NULL)
    {
        fprintf(stderr, "Cannot allocate memory for thread handles.\n");
        return false;
    }

    *args = malloc(thread_count * sizeof(solve_args));
    if (*args == NULL)
    {
        fprintf(stderr, "Cannot allocate memory for thread args.\n");
        free(*handles);
        return false;
    }

    return true;
}

void array_2d_print(size_t rows, size_t cols, double (*array)[rows][cols], FILE *stream)
{
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            fprintf(stream, "%f ", (*array)[i][j]);
        }

        fprintf(stream, "\n");
    }
}
