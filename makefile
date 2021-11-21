CPPFLAGS=-std=c++11 -Wall
DEBUG=-g
CC=g++

program2: program2.cpp
	$(CC) -o $@ $< $(CPPFLAGS)

debug: program2.cpp
	$(CC) -o $@ $< $(CPPFLAGS) $(DEBUG)

.PHONY: FCFS STRF RR
FCFS: program2
	sh FCFS.sh

STRF: program2
	sh STRF.sh

RR: program2
	sh RR_01.sh
	sh RR_02.sh

runall: FCFS STRF RR
	echo "All done!"

.PHONY: clean

clean:
	rm -f program2 debug program2_1.csv program2_2.csv program2_3_001.csv program2_3_02.csv
