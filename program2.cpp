/*
	TEAM:
	Andy Herr, jah534
	Austin Faulkner, a_f408
*/

#include <stdio.h>    // TO GET THE BASH SCRIPT(S) RUNNING
#include <stdlib.h>   // TO GET THE BASH SCRIPT(S) RUNNING
#include <iostream>
#include <cmath>
#include <list>
#include <iterator>
#include <memory> 	//shared_ptr
#include <ctime>	//For seeding rand.
#include <string>	//For parsing args.
using s_t = std::size_t;



enum EventType	{ARRIVAL, DEPARTURE, PREEMPT};
enum Algorithm	{UNKNOWN, FCFS, STRF, RR};



typedef struct Process{
	s_t		pid;
	float	timeArrived;
	float	burstLength;
	float	timeRemaining;
} Process;
using procPtr = std::shared_ptr<Process>;



//event structure
typedef struct Event{
	procPtr		relevantProcess;
	float 		time;
	EventType	type;

	Event(procPtr p, float t, EventType et)
	: relevantProcess(p)
	, time(t)
	, type(et)
	{}
} Event;
using evePtr = std::shared_ptr<Event>;



// function definition
s_t argv_length(char**);
void init(int argc, char** argv);
bool run_sim();
void generate_report();
void schedule_event(evePtr eve);
void process_arrival(evePtr eve);
void process_departure(evePtr eve);
void process_preempt(evePtr eve);
void schecule_new_arrival(bool isFirst);
float genexp(float rate);
void STRFEnqueProcess(procPtr& process);
void RRPreemptOrDepart();
void updateTurnaround();



//Global variables
//Ideally, since this is a C++ project, we would have a Simulator class
//instead of all these global variables
const s_t PROCESS_GOAL = 10000;
std::list<evePtr>		eventList{};
std::list<procPtr>		readyQueue{};
float simClock = 0.0;	//Due to lack of information, clock runs in seconds
float avgArrRate = 0.0;
float avgBurstTime = 0.0;
float quantum = 0.0;
s_t nextId = 1;
Algorithm currentAlgo = UNKNOWN;
procPtr pInProcessor = nullptr;

//Variables used for statistics
s_t numProcesses = 0;
float avgWait = 0.0;
float avgTurnaround= 0.0;
float timeSpentBusy = 0.0;
//No need to track size of ready queue, since data structure
//does that for us.



void schedule_new_arrival(bool isFirst)
{
	//First, determine the arrival time.
	float arrivalTime = simClock + (isFirst ? 0 : genexp(avgArrRate));

	//Next, generate the process
	float burstLength = genexp(1/avgBurstTime);
	auto process = procPtr(new Process{nextId++, arrivalTime, burstLength, burstLength});

	//Then, generate the event in which the process arrives.
	auto newArrival = evePtr(new Event{process, process->timeArrived, ARRIVAL});

	//Finally, add it to the event queue.
	schedule_event(newArrival);
}



void init(int argc, char** argv)
{
	//Seed the random number generator to get different data each run.
	std::srand(std::time(NULL));

	//No need to validate inputs, since program is run systematically.
	//We still need to check for zero length, though.
	if(argc == 1)	//i.e., No arguments were supplied...
	{
        std::cout << "Program takes arguments the form:\n";
		std::cout << "\t'./programName [currentAlgo] [avgArrRate] [avgBurstTime] [quantum]'\n\n";

		std::cout << "Argument explanation:\n";
		std::cout << "\t[CurrentAlgo]: The scheduling algorithm used for the current run. Only accepts values [1,3].\n";
		std::cout << "\t\t1: First-Come-First-Serve (FCFS).\n";
		std::cout << "\t\t2: Shortest-Time-Remaining-First (STRF).\n";
		std::cout << "\t\t3: Round-Robin (RR).\n\n";

		std::cout << "\t[avgArrRate]: Average arrival rate for new processes.\n\n";

		std::cout << "\t[avgBurstTime]: The average time a process runs until it finishes its burst.\n\n";

		std::cout << "\t[quantum]: The amount of time a process runs before it is preempted.\n";
		std::cout << "\t\tUsed ONLY in Round Robin.\n";

		//No arguments is a successful run.
		exit(EXIT_SUCCESS);
	}
	else
	{
		//There is no need to declare variables here;
		//there are already global ones.
		currentAlgo = static_cast<Algorithm>(atoi(argv[1]));

		avgArrRate = std::stof(argv[2]);

		avgBurstTime = std::stof(argv[3]);

		//This segment is only for Round Robin.
		//Trying to access this element for any other algo
		//will do weird stuff.
		if (currentAlgo == RR)
		{
			if (argv[4])
				quantum = std::stof(argv[4]);
			else
			{
				std::cout << "Error: must supply [quantum] argument when using Round Robin.";
				//Error.
			}
		}

		//Schedule first events here.
		bool firstArrival = true;
		schedule_new_arrival(firstArrival);
	}
}



void generate_report()
{
	// output statistics
	// Metrics to be reported:
	// 1. Average Turnaround Time
	std::cout << avgTurnaround << ",";	
	
	// 2. Total Throughput in processes/second.
	std::cout << ((float) PROCESS_GOAL) / simClock << ",";

	// 3. CPU Utilizaation
	std::cout << timeSpentBusy/simClock << ",";

	// 4. Average Ready Queue length
	s_t avgRQLength = (s_t) avgArrRate * avgWait;
	std::cout << avgRQLength << std::endl;
}



//schedules an event in the future
void schedule_event(evePtr newEvent)
{
	//Involves searching the event queue using an iterator.
	auto eventIter = eventList.begin();

	//Advance iterator until it either reaches the end,
	//or an event with a time greater than its own.
	while(eventIter != eventList.end() && newEvent->time >= (*eventIter)->time)
	{
		eventIter++;
	}

	//Insert event at this position.
	eventList.insert(eventIter, newEvent);
}



// returns a random number between 0 and 1
float urand()
{
	return( ((float) rand())/RAND_MAX );
}



// returns a random number that follows an exp distribution
float genexp(float rate)
{
	float u,x;
	x = 0;
	while (x == 0)
	{
		u = urand();
		x = (-1/rate)*log(u);
	}
	return(x);
}



//Decide whether or not the current process will depart,
//or be preempted
void RRPreemptOrDepart()
{
	//I know this right here breaks RAII,
	//but favoring `auto` here would make newEvent leave scope
	std::shared_ptr<Event> newEvent;
	if (pInProcessor->timeRemaining > quantum)
	{
		//`newEvent` is the current process's preemption in `quantum` units
		newEvent = evePtr
			(new Event{pInProcessor, simClock+quantum, PREEMPT});
	}
	else
	{
		//`newEvent` is the current process's departure in `timeRemaining` units
		newEvent = evePtr
			(new Event{pInProcessor, simClock+(pInProcessor->timeRemaining), DEPARTURE});
	}
	schedule_event(newEvent);
}



void processArrival(evePtr eve)
{
	switch (currentAlgo)
	{
		case FCFS:
			if (pInProcessor == nullptr)	// Nothing being processed
			{
				//Put the relevant process in the processor
				pInProcessor = eve->relevantProcess;

				//Use pInProcessor's Time Remaining to determine its departure time.
				auto departureTime = simClock+(pInProcessor->timeRemaining);
				evePtr newDeparture = evePtr(new Event{pInProcessor, departureTime, DEPARTURE});

				//Schedule the departure of this process.
				schedule_event(newDeparture);
			}
			else
			{
				//Put relevant process at the end of process queue.
				readyQueue.push_back(eve->relevantProcess);
			}
			break;
		case STRF:
			if (pInProcessor == nullptr)
			{
				pInProcessor = eve->relevantProcess;
			}
			else
			{
				//If new process has less time left than current process...
				if (eve->relevantProcess->timeRemaining < pInProcessor->timeRemaining)
				{
					//Schedule preemption of pInProcess at current time
					auto newEvent = std::make_shared<Event>(pInProcessor, simClock, PREEMPT);
					schedule_event(newEvent);

					//Place new process at start of readyQueue
					//so it will be processed next
					readyQueue.push_front(eve->relevantProcess);
				}
				else
				{
					//Place new process where it goes in readyQueue
					STRFEnqueProcess(eve->relevantProcess);
				}
			}
			break;
		case RR:
			if (pInProcessor == nullptr)
			{
				//Put relevant process in processor
				pInProcessor = eve->relevantProcess;

				RRPreemptOrDepart();
			}
			else
			{
				//Put relevant process at the end of process queue.
				readyQueue.push_back(eve->relevantProcess);
			}
			break;
		default:
			std::cerr << "This should have been impossible.\n";
			exit(-1);
	}
	//Schedule arrival of the next process.
	bool otherArrival = false;
	schedule_new_arrival(otherArrival);
}



void processDeparture(evePtr eve)
{
	//Do the following code for all cases
	//Update average turnaround
	float timeDeparted = simClock;
	float curTurnaround = timeDeparted - pInProcessor->timeArrived;

	float grossTurnaround = avgTurnaround * (numProcesses);
	avgTurnaround = (grossTurnaround + curTurnaround) / (numProcesses+1);

	//Update average waiting time
	float curWait = curTurnaround - pInProcessor->burstLength;
	float grossWait = avgWait * numProcesses;
	avgWait = (grossWait + curWait) / (numProcesses+1);

	//I didn't do this earlier because it would've made things weird.
	numProcesses++;

	switch (currentAlgo)
	{
		case FCFS:
			//If there are no waiting processes, then server is idle.
			if (readyQueue.empty())
			{
				pInProcessor = nullptr;
			}
			else
			{
				//Move the process at start of ready queue to the processor.
				pInProcessor = readyQueue.front();
				readyQueue.pop_front();

				//Schedule aforementioned process's departure.
				auto newEvent = evePtr(new Event {pInProcessor, simClock+(pInProcessor->timeRemaining), DEPARTURE});
				schedule_event(newEvent);
			}
			break;
		case STRF:
			//Since departure scheduling is handled in other functions,
			//only need to handle processor stuff here
			if (readyQueue.empty())
			{
				pInProcessor = nullptr;
			}
			else
			{
				//Set pInProcessor to first process in readyQueue
				pInProcessor = readyQueue.front();
				readyQueue.pop_front();
			}
			break;
		case RR:
			//If there are no waiting processes, then processor is empty
			if (readyQueue.empty())
			{
				pInProcessor = nullptr;
			}
			else
			{
				//Move first process in ready queue to the processor.
				pInProcessor = readyQueue.front();
				readyQueue.pop_front();

				//Schedule the new process's Preemption or Departure
				RRPreemptOrDepart();
			}
			break;
		default:
			std::cout << "This should have been impossible.\n";
	}
}



void processPreempt(evePtr eve)
{
	switch (currentAlgo)
	{
		case FCFS:
			std::cout << "This should have been impossible.\n";
			break;
		case STRF:
			//Place pInProcessor in appropriate spot in readyQueue.
			STRFEnqueProcess(pInProcessor);


			//Set `pInProcessor to process at front of readyQueue
			pInProcessor = readyQueue.front();
			readyQueue.pop_front();
			break;
		case RR:
			//Move `pInProcessor` to back of readyQueue
			readyQueue.push_back(pInProcessor);

			//Set `pInProcessor` to first process in `readyQueue`
			pInProcessor = readyQueue.front();
			readyQueue.pop_front();

			RRPreemptOrDepart();
			break;
		default:
			std::cout << "This should have been impossible.\n";
	}
}



//Return true if simulation ended successfully.
//Return False if something weird happens.
bool run_sim()
{
	while (numProcesses < PROCESS_GOAL)
	{
		if (eventList.empty())
		{
			std::cerr << "Ran out of events.\n";
			return false;
			break;
		}

		float timeDifference = eventList.front()->time - simClock;
		//STRF STUFF
		if (currentAlgo == STRF)
		{
			//If the current process will end before the next event
			if ((pInProcessor) && pInProcessor->timeRemaining < timeDifference)
			{
				//Schedule departure event for the time at which pInProcessor will end
				auto pDeparture = std::make_shared<Event>
					(pInProcessor, simClock+(pInProcessor->timeRemaining), DEPARTURE);
				schedule_event(pDeparture);

				//At this point, since a new event was added,
				//we need to calculate the time difference again.
				//Thankfully, we already know that it's timeRemaining.
				timeDifference = pInProcessor->timeRemaining;
			}
		}

		auto eve = eventList.front();
		eventList.pop_front();
		if (pInProcessor)
		{
			pInProcessor->timeRemaining -= timeDifference;
			timeSpentBusy += timeDifference;
		}

    	simClock = eve->time;
		switch (eve->type)
		{
			case ARRIVAL:
				processArrival(eve);
				break;
			case DEPARTURE:
				processDeparture(eve);
				break;
			case PREEMPT:
				processPreempt(eve);
				break;
			default:
				std::cerr << "Error.\n";
		}
    }
	//If code reaches this point, simulation finished successfully.
	return true;
}



void STRFEnqueProcess(procPtr& process)
{
	auto rQIter = readyQueue.begin();

	//Iterate until you find a process with MORE time left than the target process
	while ((rQIter != readyQueue.end()) && ((*rQIter)->timeRemaining <= process->timeRemaining))
	{
		rQIter++;
	}
	readyQueue.insert(rQIter, process);
}



int main(int argc, char *argv[])
{
	// parse arguments:
	init(argc, argv);

	bool simSuccess = run_sim();
	if (simSuccess)
	{
		generate_report();
	}
	else
	{
	 	std::cout << "Something went wrong.\n";
		return -1;
	}

	return 0;
}
