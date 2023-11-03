#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#include "relaxation.h"

void testcase_1()
{
    printf("testcase 1:\n");
    size_t size = 4;
    double test_values[4][4] = {{1.0, 1.0, 1.0, 1.0},
                                {1.0, 0.0, 0.0, 0.0},
                                {1.0, 0.0, 0.0, 0.0},
                                {1.0, 0.0, 0.0, 0.0}};

    run(size, &test_values);
}

void testcase_2()
{
    printf("testcase 2:\n");
    size_t size = 10;
    double test_values[10][10] = {{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                  {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

    run(size, &test_values);
}

void testcase_3()
{
    printf("testcase 3:\n");
    static const size_t size = 100;
    double test_values[size][size];

    for (size_t j = 0; j < size; j++)
        test_values[0][j] = 1.0f;

    for (size_t i = 1; i < size; i++)
    {
        test_values[i][0] = 1.0f;
        for (size_t j = 1; j < size; j++)
            test_values[i][j] = 0.0f;
    }

    for (size_t j = 0; j < size; j++)
        test_values[j][size - 1] = 1.0f;

    run(size, &test_values);
}

int main(void)
{
    static const size_t size = 10;
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

    solve(size, buf, 1, 0.01);

    free(buf);
    return 0;
}

void run(size_t size, double (*values)[size][size])
{
    double(*buf)[size][size];
    array_2d_alloc_no_err(size, size, &buf);
    memcpy(buf, values, sizeof(*buf));

    solve(size, buf, 1, 0.01);

    free(buf);
}

// TODO: malloc functions for error andling; refactor args array as single 2d array (single free)
void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision)
{
    thread_count = (size < thread_count ? size : thread_count);
    const size_t rows_per_thread = size / thread_count;
    const size_t rows_last_thread = size % thread_count;

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
        args[i].start_row = 1 + i * rows_per_thread;
        args[i].end_row = args[i].start_row + (i == thread_count - 1 ? rows_last_thread : rows_per_thread);
        *(args[i].converged) = true;
    }

    size_t iterations = 0;
    bool converged = false;
    while (!converged)
    {
        for (size_t i = 0; i < thread_count; i++)
        {
            *(args[i].converged) = true;
            args[i].values = (double *)values;
            args[i].prev_values = (double *)prev_values;
            pthread_create(&handles[i], NULL, (void *(*)(void *))solve_chunk, &args[i]);
        }

        for (size_t i = 0; i < thread_count; i++)
        {
            pthread_join(handles[i], NULL);
        }

        ++iterations;
        printf("iteration: %ld\n", iterations);
        array_2d_print(size, size, values, stdout);
        memcpy(prev_values, values, sizeof(*prev_values));

        converged = true;
        for (size_t i = 0; i < thread_count; i++)
        {
            if (*(args->converged) == false)
            {
                converged = false;
                break;
            }
        }
    }
}

void *solve_chunk(solve_args *args)
{
    double(*values)[args->size][args->size] = (double(*)[args->size][args->size])args->values;
    double(*prev_values)[args->size][args->size] = (double(*)[args->size][args->size])args->prev_values;

    for (size_t row = args->start_row; row < args->end_row - 1; row++)
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
    while (!converged)
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
        printf("iteration: %ld\n", iterations);
        // array_2d_print(size, size, values, stdout);
        memcpy(prev_values, values, sizeof(*prev_values));
    }

    FILE *file = fopen("log", "w");
    if (file)
    {
        array_2d_print(size, size, values, file);
        fclose(file);
    }
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

    bool *converged_flags = malloc(thread_count * sizeof(bool));
    if (converged_flags == NULL)
    {
        fprintf(stderr, "Cannot allocate memory for convergence_flags.\n");
        free(*handles);
        free(*args);
        return false;
    }

    for (size_t i = 0; i < thread_count; i++)
    {
        (*args)[i].converged = &converged_flags[i];
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
