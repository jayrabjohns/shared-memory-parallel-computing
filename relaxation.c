#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

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

int main(void)
{
    testcase_1();
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

void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision)
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
        array_2d_print(size, size, *values);
        memcpy(prev_values, values, sizeof(*prev_values));
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

void array_2d_print(size_t rows, size_t cols, double array[rows][cols])
{
    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            printf("%f ", array[i][j]);
        }

        printf("\n");
    }
}

// void panic(const char *error_str)
// {
//     if (error_str)
//         fprintf(stderr, "\n%s", error_str);

//     exit(1);
// }

// void panic_malloc()
// {
//     panic("Cannot allocate memory.\n");
// }