# CPU Scheduling Mechanics in Operating Systems

"The objective of multiprogramming is to have some process running at all times, to maximize CPU utilization. [...] A process is exectured until it must wait, typically for the completion of some I/O request. [...] When on process has to wait, the operating system (OS) takes the CPU away from the process and gives the CPU to another process. [...] On a multicore system, this concept of keeping the system busy is extended to all processing cores on the system. 

Scheduling of this kind is a fundamental OS function. Almost all computer resources are scheduled before use."

Operating System Concepts 10th Edition, Seilberschatz, et al.

# Running This Program

- Run the program by first running 'make runall'. This command will compile
  program2.cpp and then run all bash scripts.

- To clean up the folder, run 'make clean'. This command will remove the
  executable 'program2' and all bash scripts.

- If you choose you can then run 'make runall' to see another trial of the
  Excel compatible .csv output.

# A Detailed Analysis of 3 Scheduling Algorithms

The algorithms discussed are the Shortest-Remaining-Time-First (SRTF) algorithm, the First-Come-First-Serve (FCFS) algorithm, and the Round Robin (RR) algorithm with lambda = 0.01 and lambda = 0.2, respectively.

[CPUSchedulingAnalysis.pdf](https://github.com/Austin-Faulkner/Discrete-Time_Event_Simulator_OSs/files/8351835/CPUSchedulingAnalysis.pdf)

