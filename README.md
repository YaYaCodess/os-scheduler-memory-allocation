# OS Scheduler and Memory Allocation

This is a student-made Operating Systems project implemented in C on Linux. The project simulates CPU process scheduling using inter-process communication and extends the scheduler with buddy memory allocation.

## Features

* Process generator that reads process data from an input file
* Simulated system clock using shared memory
* Scheduler process using IPC message queues and signals
* Process Control Block (PCB) tracking for each process
* CPU scheduling algorithms:

  * Shortest Job First (SJF)
  * Preemptive Highest Priority First (HPF)
  * Round Robin (RR)
  * Multilevel Feedback Queue (MLFQ)
* Buddy memory allocation system with 1024-byte memory
* Scheduler performance logging
* Memory allocation and deallocation logging

## Technologies Used

* C
* Linux
* IPC Message Queues
* Shared Memory
* Signals
* Process Forking
* Buddy Memory Allocation

## Project Files

```txt
process_generator.c   # Reads processes and sends them to scheduler
scheduler.c           # Main scheduling and memory allocation logic
process.c             # Simulated CPU-bound process
clk.c                 # Simulated system clock
headers.h             # Shared definitions and IPC helpers
pcb.h                 # Process Control Block structure
mem.h                 # Buddy memory allocation implementation
pri_queue.h           # Priority queue implementation
doubly_linked_list.h  # Linked list implementation
processes.txt         # Sample input file
```

## How to Compile

```bash
gcc clk.c -o clk.out
gcc process.c -o process.out
gcc scheduler.c -o scheduler.out -lm
gcc process_generator.c -o process_generator.out
```

## How to Run

```bash
./process_generator.out processes.txt -sch 1
```

For Round Robin or MLFQ, include the quantum value:

```bash
./process_generator.out processes.txt -sch 3 -q 2
```

## Scheduling Algorithm Numbers

```txt
1 = SJF
2 = HPF
3 = RR
4 = MLFQ
```

## Output Files

The program generates:

```txt
scheduler.log   # Process scheduling events
scheduler.perf  # CPU utilization, average WTA, waiting time, and standard deviation
memory.log      # Buddy memory allocation and freeing events
```

## Notes

This project was developed for educational purposes as part of an Operating Systems course. It demonstrates process scheduling, IPC, process management, performance calculation, and memory allocation concepts.
