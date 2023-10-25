#ifndef RELAXATION_H
#define RELAXATION_H

#include <stdio.h>
#include <stdlib.h>

void test() { printf("test."); };
void solve(double **values, unsigned int size, unsigned int thread_count, double precision);

void panic(const char *error_str)
{
    if (error_str)
    {
        fprintf(stderr, "\n%s", error_str);
    }
    exit(1);
}

void panic_malloc()
{
    panic("Cannot allocate memory.\n");
}

#endif