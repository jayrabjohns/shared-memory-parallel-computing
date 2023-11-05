#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#include "relaxation.h"

// ANSI escape code which moves cursor to beginning of previous line.
// Supported by Unix & Windows 10+
static const char RESET_LINE[] = "\033[A";

/* Creates a nxn matrix of the form
   [1 1 ... 1]
   [1 0 ... 0]
   [.........]
   [1 0 ... 0]
 */
void load_testcase_1(size_t size, double (*matrix)[size][size])
{
    for (size_t j = 0; j < size; j++)
    {
        (*matrix)[0][j] = 1.0f;
    }

    for (size_t i = 1; i < size; i++)
    {
        (*matrix)[i][0] = 1.0f;
        for (size_t j = 1; j < size; j++)
        {
            (*matrix)[i][j] = 0.0f;
        }
    }
}

void load_testcase_2(size_t size, double (*matrix)[size][size])
{
    for (size_t j = 0; j < size; j++)
    {
        (*matrix)[0][j] = 1.0f;
    }

    for (size_t i = 1; i < size; i++)
    {
        (*matrix)[i][0] = 1.0f;
        for (size_t j = 1; j < size; j++)
        {
            (*matrix)[i][j] = 0.0f;
        }
    }
}

int main(void)
{
    static const size_t size = 10000;
    static const double precision = 0.001;
    static const size_t thread_count = 8;
    printf("matrix size: %ld\nprecision: %f\nthread count:%ld\n",
           size, precision, thread_count);

    int rc = 0;
    double(*result_async)[size][size];
    rc = array_2d_try_alloc(size, &result_async);
    if (rc != 0)
        return rc;

    load_testcase_1(size, result_async);
    rc = solve(size, result_async, thread_count, precision);
    if (rc != 0)
        return rc;

    double(*result_sync)[size][size];
    rc = array_2d_try_alloc(size, &result_sync);
    if (rc != 0)
        return rc;

    load_testcase_1(size, result_sync);
    solve_sync(size, result_sync, precision);

    rc = memcmp(result_async, result_sync, sizeof(*result_async));
    if (rc == 0)
    {
        printf("PASS solution matches synchronous implementation\n");
    }
    else
    {
        printf("FAIL solution doesn't match synchronous implementation\n");
        printf("\nsync impl result\n");
        array_2d_print(size, result_sync, stdout);
        printf("async impl result:\n");
        array_2d_print(size, result_async, stdout);
    }

    free(result_sync);
    free(result_async);

    return rc;
}

int solve(size_t size, double (*values)[size][size], size_t thread_count,
          double precision)
{
    thread_count = (size < thread_count ? size : thread_count);
    const size_t rows_per_thread = (size - 2) / thread_count;
    // With this way of assigning rows per thread, there's an edge case where
    // (thread_count - 2) > size and all work get delegates the final thread
    // (since the division gets rounded to 0).
    // This should be fine, because in any reasonable case where this would be
    // noticeable, the size of the problem will be far higher than the number
    // of cores available (asusming one thread per core)

    int rc;
    double(*prev_values)[size][size];
    pthread_t *handles;
    solve_args *args;

    rc = solve_try_alloc(size, &prev_values, thread_count, &handles, &args);
    if (rc != 0)
        return rc;

    memcpy(prev_values, values, sizeof(*prev_values));
    for (size_t i = 0; i < thread_count; i++)
    {
        args[i].size = size;
        args[i].values = (double *)values;
        args[i].prev_values = (double *)prev_values;
        args[i].start_row = i * rows_per_thread + 1;
        if (i == thread_count - 1)
            args[i].end_row = size - 1;
        else
            args[i].end_row = args[i].start_row + rows_per_thread;

        printf("thread: %ld; start_row: %ld; end_row: %ld\n",
               i, args[i].start_row, args[i].end_row);
    }
    printf("\n");

    size_t iterations = 0;
    bool converged = false;
    time_t start, now;
    double elapsed_time;
    time(&start);
    while (!converged)
    {
        for (size_t i = 0; i < thread_count; i++)
        {
            args[i].values = (double *)values;
            args[i].prev_values = (double *)prev_values;
            void *(*thunk)(void *) = (void *(*)(void *))solve_chunk;
            pthread_create(&handles[i], NULL, thunk, &args[i]);
        }

        for (size_t i = 0; i < thread_count; i++)
        {
            pthread_join(handles[i], NULL);
        }

        converged = matrix_has_converged(precision, size, values, prev_values);
        memcpy(prev_values, values, sizeof(*prev_values));
        ++iterations;

        time(&now);
        elapsed_time = difftime(now, start);
        printf("%s[ASYNC] iteration: %ld (%.0lfs)\n",
               RESET_LINE, iterations, elapsed_time);
    }

    printf("[ASYNC] solved in %ld iterations in %.0lfs\n",
           iterations, elapsed_time);

    free(prev_values);
    free(handles);
    free(args);
    return 0;
}

void *solve_chunk(solve_args *args)
{
    size_t size = args->size;
    double(*values)[size][size] = (double(*)[size][size])args->values;
    double(*prev_values)[size][size] = (double(*)[size][size])args->prev_values;

    for (size_t row = args->start_row; row < args->end_row; row++)
    {
        for (size_t col = 1; col < args->size - 1; col++)
        {
            const double neighbours_sum =
                (*prev_values)[row - 1][col] +
                (*prev_values)[row + 1][col] +
                (*prev_values)[row][col - 1] +
                (*prev_values)[row][col + 1];
            (*values)[row][col] = neighbours_sum / 4.0;
        }
    }

    return NULL;
}

int solve_sync(size_t size, double (*values)[size][size], double precision)
{
    int rc = 0;
    double(*prev_values)[size][size];
    rc = array_2d_try_alloc(size, &prev_values);
    if (rc != 0)
        return rc;

    memcpy(prev_values, values, sizeof(*prev_values));
    printf("\n");

    size_t iterations = 0;
    bool converged = false;
    time_t start, now;
    double elapsed_time;
    time(&start);
    while (!converged)
    {
        for (size_t row = 1; row < size - 1; row++)
        {
            for (size_t col = 1; col < size - 1; col++)
            {
                double neighbours_sum =
                    (*prev_values)[row - 1][col] +
                    (*prev_values)[row + 1][col] +
                    (*prev_values)[row][col - 1] +
                    (*prev_values)[row][col + 1];
                (*values)[row][col] = neighbours_sum / 4.0;
            }
        }

        converged = matrix_has_converged(precision, size, values, prev_values);
        memcpy(prev_values, values, sizeof(*prev_values));
        ++iterations;

        time(&now);
        elapsed_time = difftime(now, start);
        printf("%s[SYNC] iteration: %ld (%.0lfs)\n",
               RESET_LINE, iterations, elapsed_time);
    }

    printf("[SYNC] solved in %ld iterations in %.0lfs\n",
           iterations, elapsed_time);

    free(prev_values);

    return rc;
}

bool matrix_has_converged(
    double precision, size_t size,
    const double (*m1)[size][size], const double (*m2)[size][size])
{
    bool has_converged = true;
    for (size_t row = 0; row < size && has_converged; row++)
    {
        for (size_t col = 0; col < size && has_converged; col++)
        {
            double diff = fabs((*m1)[row][col] - (*m2)[row][col]);
            if (diff > precision)
                has_converged = false;
        }
    }
    return has_converged;
}

int solve_try_alloc(size_t size, double (**matrix)[size][size],
                    size_t thread_count, pthread_t **handles, solve_args **args)
{
    int rc = 0;
    *handles = malloc(thread_count * sizeof(pthread_t));
    if (*handles == NULL)
    {
        fprintf(stderr, "Cannot allocate memory for thread handles.\n");
        rc = 1;
    }

    *args = malloc(thread_count * sizeof(solve_args));
    if (*args == NULL)
    {
        fprintf(stderr, "Cannot allocate memory for thread args.\n");
        free(*handles);
        rc = 1;
    }

    rc = array_2d_try_alloc(size, matrix);
    if (rc != 0)
    {
        free(args);
        free(handles);
    }

    return rc;
}

int array_2d_try_alloc(size_t size, double (**values)[size][size])
{
    *values = malloc(size * size * sizeof(double));
    if (*values == NULL)
    {
        size_t size_in_gb = size * size * sizeof(double) / 1000000000;
        fprintf(stderr, "Cannot allocate memory for %ldx%ld array. (%ldGB)\n",
                size, size, size_in_gb);
        return 1;
    }

    return 0;
}

void array_2d_print(size_t size, double (*array)[size][size], FILE *stream)
{
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            fprintf(stream, "%f ", (*array)[i][j]);
        }

        fprintf(stream, "\n");
    }
}
