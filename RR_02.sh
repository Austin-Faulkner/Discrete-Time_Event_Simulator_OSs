#!/bin/bash

rm -f program2_3_02.csv

for ((i = 1; i <= 30; i++)); do
   # sim3.2 3 = FCFS, $i = lambda, 0.06 = avg. burst, quantum = 0.2
   ./program2 3 $i 0.06 0.2

done > program2_3_02.csv # outfile name
truncate -s -1 program2_3_02.csv
