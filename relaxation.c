#include <stdio.h>
#include <stdlib.h>

#include "relaxation.h"

void alloc_values();
void free_values();

int main(void)
{
    unsigned int size = 4;
    double **values = malloc(size * sizeof(double *));
    if (values == NULL)
    {
        panic_malloc();
    }

    for (size_t i = 0; i < size; ++i)
    {
        values[i] = malloc(size * sizeof(double));
        if (values == NULL)
        {
            panic_malloc();
        }
    }

    // solve();

    for (size_t i = 0; i < size; ++i)
    {
        free(values[i]);
    }
    free(values);
    return 0;
}

void solve(double **values, unsigned int size, unsigned int thread_count, double precision)
{
    double x = **values + size + thread_count + precision;
    ++x;
    return;
}