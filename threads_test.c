#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

typedef struct thunk_args thunk_args;

struct thunk_args
{
    size_t size;
    double *values;
    // double *prev_values;
    // double precision;
    // size_t start_row;
    // size_t end_row;
    // bool converged;
};

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

void thunk(thunk_args *args)
{
    printf("executing from inside a thread:\n");
    array_2d_print(args->size, args->size, (double(*)[args->size][args->size])args->values, stdout);
}

int main(void)
{
    const static size_t size = 5;

    double(*values)[size][size];
    values = malloc(size * size * sizeof(double));
    for (size_t j = 0; j < size; j++)
        (*values)[0][j] = 1.0f;

    for (size_t i = 1; i < size; i++)
    {
        (*values)[i][0] = 1.0f;
        for (size_t j = 1; j < size; j++)
            (*values)[i][j] = 0.0f;
    }

    pthread_t *handles = malloc(size * sizeof(pthread_t));
    thunk_args *args = malloc(size * sizeof(thunk_args));
    for (size_t i = 0; i < size; i++)
    {
        args[i] = (thunk_args){size, (double *)values};
        // args[i].size = size;
        // args[i].values = (double *)values;
    }

    for (size_t i = 0; i < size; i++)
    {
        pthread_t *handle = &handles[i];
        pthread_create(handle, NULL, (void *(*)(void *))thunk, &args[i]);
    }

    for (size_t i = 0; i < size; i++)
    {
        pthread_join(handles[i], NULL);
    }

    free(args);
    free(handles);
}
