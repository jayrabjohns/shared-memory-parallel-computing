valgrind detected an error
this was the same error that was causing the async implementation to always take one iteration more.
```
==116944== Memcheck, a memory error detector
==116944== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==116944== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==116944== Command: ./relaxation
==116944== 
matrix size: 1000
precision: 0.010000
thread count:8
thread: 0; start_row: 1; end_row: 125
thread: 1; start_row: 125; end_row: 249
thread: 2; start_row: 249; end_row: 373
thread: 3; start_row: 373; end_row: 497
thread: 4; start_row: 497; end_row: 621
thread: 5; start_row: 621; end_row: 745
thread: 6; start_row: 745; end_row: 869
thread: 7; start_row: 869; end_row: 999

==116944== Conditional jump or move depends on uninitialised value(s)
==116944==    at 0x10A71C: matrix_has_converged (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x109D03: solve (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x10967D: main (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==  Uninitialised value was created by a heap allocation
==116944==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==116944==    by 0x10A95D: array_2d_try_alloc (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x10A8B0: solve_try_alloc (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x1099B9: solve (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x10967D: main (in /home/jay/src/shared-memory-parallel-computing/relaxation)
[ASYNC] iteration: 38 (23s)
[ASYNC] solved in 38 iterations in 23s
[SYNC] iteration: 37 (7s)
[SYNC] solved in 37 iterations in 7s
==116944== Conditional jump or move depends on uninitialised value(s)
==116944==    at 0x485207E: bcmp (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==116944==    by 0x10978F: main (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==  Uninitialised value was created by a heap allocation
==116944==    at 0x4848899: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==116944==    by 0x10A95D: array_2d_try_alloc (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x10A8B0: solve_try_alloc (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x1099B9: solve (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944==    by 0x10967D: main (in /home/jay/src/shared-memory-parallel-computing/relaxation)
==116944== 
PASS solution matches synchronous implementation
==116944== 
==116944== HEAP SUMMARY:
==116944==     in use at exit: 0 bytes in 0 blocks
==116944==   total heap usage: 163 allocs, 163 frees, 32,043,840 bytes allocated
==116944== 
==116944== All heap blocks were freed -- no leaks are possible
==116944== 
==116944== For lists of detected and suppressed errors, rerun with: -s
==116944== ERROR SUMMARY: 2032028 errors from 2 contexts (suppressed: 0 from 0)
```

``` C
double (*prev_values)[size][size];
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
```

### NEW 
``` C
double (*prev_values)[size][size];
pthread_t *handles;
solve_args *args;

rc = solve_try_alloc(size, &prev_values, thread_count, &handles, &args);
if (rc != 0)
    return rc;
```



movesthe logic from array_2d_alloc to a new function along with the logic from try_alloc_thread_handlers
forgets to memcpy in the values from values into prev-Values.


### FIX
``` C
double(*prev_values)[size][size];
pthread_t *handles;
solve_args *args;

rc = solve_try_alloc(size, &prev_values, thread_count, &handles, &args);
if (rc != 0)
    return rc;

memcpy(prev_values, values, sizeof(*prev_values));
```

### Valgrind after
==117768== Memcheck, a memory error detector
==117768== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==117768== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==117768== Command: ./relaxation
==117768== 
matrix size: 1000
precision: 0.010000
thread count:8
thread: 0; start_row: 1; end_row: 125
thread: 1; start_row: 125; end_row: 249
thread: 2; start_row: 249; end_row: 373
thread: 3; start_row: 373; end_row: 497
thread: 4; start_row: 497; end_row: 621
thread: 5; start_row: 621; end_row: 745
thread: 6; start_row: 745; end_row: 869
thread: 7; start_row: 869; end_row: 999
[ASYNC] iteration: 37 (19s)
[ASYNC] solved in 37 iterations in 19s
[SYNC] iteration: 37 (7s)
[SYNC] solved in 37 iterations in 7s
PASS solution matches synchronous implementation
==117768== 
==117768== HEAP SUMMARY:
==117768==     in use at exit: 0 bytes in 0 blocks
==117768==   total heap usage: 159 allocs, 159 frees, 32,042,752 bytes allocated
==117768== 
==117768== All heap blocks were freed -- no leaks are possible
==117768== 
==117768== For lists of detected and suppressed errors, rerun with: -s
==117768== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
