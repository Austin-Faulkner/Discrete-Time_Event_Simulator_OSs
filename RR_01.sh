#!/bin/bash

rm -f program2_3_001.csv

for ((i = 1; i <= 30; i++)); do
   # sim3.1 3 = RR, $i = lambda, 0.06 = avg. burst time, quantum = 0.01
   ./program2 3 $i 0.06 0.01

done > program2_3_001.csv  # outfile name
truncate -s -1 program2_3_001.csv
