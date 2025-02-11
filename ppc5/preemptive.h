#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadExit(void);
void myTimer0Handler(void);
void CarParking(void);
void delay(unsigned char n);
unsigned char now(void);

#endif
