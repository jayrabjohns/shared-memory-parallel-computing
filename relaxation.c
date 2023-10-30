#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    printf("Before:\n");
    array_2d_print(size, size, *buf);

    solve(size, buf, 1, 0.1);
    printf("After:\n");
    array_2d_print(size, size, *buf);

    free(buf);
}

void solve(size_t size, double (*values)[size][size], size_t thread_count, double precision)
{
    ++precision;
    ++thread_count;

    double(*values_copy)[size][size];
    array_2d_alloc_no_err(size, size, &values_copy);
    memcpy(values_copy, values, sizeof(*values_copy));

    for (size_t row = 1; row < size - 1; row++)
    {
        for (size_t col = 1; col < size - 1; col++)
        {
            double neighbours_sum = (*values_copy)[row - 1][col] + (*values_copy)[row + 1][col] + (*values_copy)[row][col - 1] + (*values_copy)[row][col + 1];
            (*values)[row][col] = neighbours_sum / 4.0;
        }
    }

    return;
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