```
==118976== Helgrind, a thread error detector
==118976== Copyright (C) 2007-2017, and GNU GPL'd, by OpenWorks LLP et al.
==118976== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==118976== Command: ./relaxation
==118976== 
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
[ASYNC] iteration: 37 (20s)
[ASYNC] solved in 37 iterations in 20s
[SYNC] iteration: 37 (11s)
[SYNC] solved in 37 iterations in 11s
PASS solution matches synchronous implementation
==118976== 
==118976== Use --history-level=approx or =none to gain increased speed, at
==118976== the cost of reduced accuracy of conflicting-access information
==118976== For lists of detected and suppressed errors, rerun with: -s
==118976== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

```