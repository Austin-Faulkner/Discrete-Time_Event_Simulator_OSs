#!/bin/bash

rm -f program2_1.csv

for ((i = 1; i <= 30; i++)); do  # C-style for-loop
   # COMMENT: sim1 1 = FCFS, $i = lambda, 0.06 = avg. burst
   ./program2 1 $i 0.06

done > program2_1.csv # outfile name
truncate -s -1 program2_1.csv
