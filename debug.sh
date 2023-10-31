#!/bin/bash
clear
cc -g3 -Wall -Wextra -Wconversion -o relaxation relaxation.c
gdb --annotate=3 relaxation
