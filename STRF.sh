#!/bin/bash

rm -f program2_2.csv

for ((i = 1; i <= 30; i++)); do  # C-style for-loop
   # COMMENT: sim1 1 = FCFS, $i = lambda, 0.06 = avgBurst
   ./program2 2 $i 0.06

done > program2_2.csv # outfile name
truncate -s -1 program2_2.csv
