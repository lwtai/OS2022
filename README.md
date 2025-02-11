# Operating Systems Programming Projects

## Course Information
- **Course**: CS 342302 Operating Systems
- **Semester**: Fall 2022
- **Professor**: Prof. Pai H. Chou

## Development Environment
- **Target Platform**: EdSim51 (8051 Microcontroller Simulator)
- **Compiler**: SDCC (Small Device C Compiler)
- **Programming Language**: C

## Project Overview
This repository contains five programming project checkpoints that implement various operating system concepts on the 8051 microcontroller platform.

### Checkpoint 1: Cooperative Multithreading
- Implementation of a basic cooperative multithreading system
- Key components:
  - Thread creation and management
  - Context switching
  - Thread yield functionality
- Test case using a producer-consumer pattern
- Implementation of thread APIs: ThreadCreate, ThreadYield

### Checkpoint 2: Preemptive Multithreading
- Extension of Checkpoint 1 to support preemptive multithreading
- Added features:
  - Timer-based preemption using Timer0
  - Interrupt handling
  - Context saving and restoration
- Same producer-consumer test case but without explicit yields

### Checkpoint 3: Semaphores
- Implementation of semaphore primitives for thread synchronization
- Features:
  - SemaphoreCreate, SemaphoreWait, and SemaphoreSignal operations
  - Busy-wait implementation
  - Thread synchronization in bounded buffer problem
- Test case using classical bounded-buffer with 3-slot buffer

### Checkpoint 4: Multiple Producers
- Extended bounded-buffer implementation with multiple producers
- Features:
  - Two producers (generating letters and numbers)
  - One consumer
  - Testing thread fairness
  - Competition handling between producers

### Checkpoint 5: Delay and Thread Management
- Implementation of timing and thread lifecycle management
- Key features:
  - delay(n) function implementation
  - now() function for system time
  - Robust thread termination
  - Thread recycling
- Parking lot simulation as test case
  - 5 cars competing for 2 parking spots
  - Event logging system
  - Thread resource management

## Building and Running
1. Use the provided Makefile in each checkpoint directory
2. Clean previous builds:
   ```bash
   make clean
   ```
3. Compile the project:
   ```bash
   make
   ```
4. Load the generated .hex file into EdSim51 simulator

## Special Notes
- Each checkpoint builds upon the previous one
- Thread limit of 4 threads in the system
- Register banks and memory locations are specifically assigned for thread management
- SDCC compiler specifics need to be considered for assembly code generation
