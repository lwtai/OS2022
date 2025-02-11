#include <8051.h>
#include "preemptive.h"

// thread delay
__data __at(0x29) unsigned static char sleepTime[MAXTHREADS];
// timer-0 counter
__data __at(0x2D) unsigned static char displayTime0;
__data __at(0x4B) unsigned static char displayTime1;
__data __at(0x2E) unsigned static char timer0Counter;
__data __at(0x7A) unsigned static char timer0Counter2;
// thread manager
__data __at(0x2F) static char constZero;
__data __at(0x30) static char threadBitMap;
__data __at(0x31) static char currentThreadId;
__data __at(0x32) static char savedStackPointer[MAXTHREADS];
// in ThreadCreate
__data __at(0x36) static char bitToSet;
__data __at(0x37) static char selectedThreadId;
__data __at(0x38) static char tempVar;
__data __at(0x39) static char tempSp;
// in ThreadExit
__data __at(0x3A) static char bitToMask;
__data __at(0x3B) static char tempVar2;
__data __at(0x3C) static char nextThread;
__data __at(0x3D) static char nextBitToSet;
// in timerISR
__data __at(0x3E) static char tempVar3;
__data __at(0x3F) static char nextThread2;


#define SAVESTATE { \
    __asm           \
        PUSH ACC    \
        PUSH B      \
        PUSH DPL    \
        PUSH DPH    \
        PUSH PSW    \
    __endasm;       \
    savedStackPointer[currentThreadId] = SP; \
}   
#define RESTORESTATE {      \
    SP = savedStackPointer[currentThreadId];  \
    __asm                   \
        POP PSW             \
        POP DPH             \
        POP DPL             \
        POP B               \
        POP ACC             \
    __endasm;               \
}
extern void main(void);

void Bootstrap(void) {

    // thread manager
    threadBitMap = 0;
    constZero = 0;
    savedStackPointer[0] = 0x3F;
    savedStackPointer[1] = 0x4F;
    savedStackPointer[2] = 0x5F;
    savedStackPointer[3] = 0x6F;

    // delay table
    sleepTime[0] = 0;
    sleepTime[1] = 0;
    sleepTime[2] = 0;
    sleepTime[3] = 0;

    // timer-0 setup
    __asm
        MOV TH0, 0x1C
        MOV TL0, 0x18
    __endasm;
    TMOD = 0; // set mode
    IE = 0x82;
    TR0 = 1;  // run timer0

    // document time
    timer0Counter = 0;
    timer0Counter2 = 0;
    displayTime0 = 0;
    displayTime1 = 0;

    // create thread main
    currentThreadId = ThreadCreate(main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp){
    
    // wait when full
    while(threadBitMap == 0x0F);
    
    EA = 0;

    char bitToSet = 0x00;
    ThreadID selectedThreadId = 0;

    // update bitmap
    bitToSet = ((threadBitMap+1) & (~threadBitMap));
    threadBitMap |= bitToSet;

    // calculate thread ID
    while (bitToSet >>= 1) {
        selectedThreadId++;
    }

    // initial the stack of thread
    tempVar = savedStackPointer[selectedThreadId];
    tempSp = SP; 
    SP = tempVar;

    // new psw value
    tempVar = (PSW & 0xE7);
    tempVar |= (selectedThreadId << 3);

    __asm   
        PUSH DPL             ;; low-byte  of fp parameter
        PUSH DPH             ;; high-byte of fp parameter
        PUSH _constZero      ;; ACC
        PUSH _constZero      ;; B 
        PUSH _constZero      ;; DPL
        PUSH _constZero      ;; DPH
        PUSH _tempVar        ;; PSW 
    __endasm;

    savedStackPointer[selectedThreadId] = SP;
    SP = tempSp; // restore original sp

    // end of critical section
    EA = 1;

    return selectedThreadId;
}

void ThreadExit(void) {

    EA = 0;

    bitToMask = 0x01;
    tempVar2 = currentThreadId;

    while (tempVar2) {
        bitToMask <<= 1;
        tempVar2 -= 1;
    }

    // updata bitmap
    bitToMask = ~bitToMask;
    threadBitMap &= bitToMask;

    // calculate new stack pointer
    tempVar2 = currentThreadId;
    tempVar2 += 0x03;
    tempVar2 <<= 4;
    tempVar2 |= 0x0F;
    // clear the space
    savedStackPointer[currentThreadId] = tempVar2;
    
    // all threads exit
    if (threadBitMap == 0x00) {
        currentThreadId = 0xFF;
        while (1);
    }

    // select next thread
    nextBitToSet = (char)0x01 << currentThreadId;
    char nextThread = currentThreadId;

    do {
        // RR
        if (nextBitToSet == (char)0x08) {
            nextBitToSet = 0x01;
            nextThread = 0;
        } else {
            nextBitToSet <<= 1;
            nextThread++;
        }

        // check if active
        if (nextBitToSet & threadBitMap) {
            currentThreadId = nextThread;
            break;
        }
    }while(1);

    RESTORESTATE;

    EA = 1;

}

void delay(unsigned char n) {
    EA = 0;

    __asm
        MOV A, 0x31
        ADD A, #0x29
        MOV R0, A
        MOV @R0, 0x82  ;; save sleepTime to array
        CLR TR0
        MOV TH0, 0x1C
        MOV TL0, 0x18
        SETB TR0
    __endasm;
    
    EA = 1;

    __asm
        Wait:
        MOV A, 0x31
        ADD A, #0x29
        MOV R1, A
        MOV A, @R1  ;; check sleepTime
        JNZ Wait    ;; wait if sleepTime > 0
    __endasm;
}

void myTimer0Handler(void) {

    // thread switching
    EA = 0;
    SAVESTATE;
    tempVar3 = (char)0x01 << currentThreadId;
    char nextThread2 = currentThreadId;

    do {
        // RR
        if (tempVar3 == (char)0x08) {
            tempVar3 = 0x01;
            nextThread2 = 0;
        } else {
            tempVar3 <<= 1;
            nextThread2++;
        }

        // check if active
        if (tempVar3 & threadBitMap) {
            currentThreadId = nextThread2;
            break;
        }
    }while(1);

    // timer
    timer0Counter++;
    timer0Counter2++;
    if (timer0Counter2 == 200) {
        timer0Counter2 = 0;
        displayTime1++;
    }
    if (timer0Counter == 20) {
        // time unit: 20ms
        timer0Counter = 0;
        displayTime0++;
        if (displayTime0 == 10) {
            displayTime0 = 0;
        }

        if (sleepTime[0] > 0) { sleepTime[0]--; }
        if (sleepTime[1] > 0) { sleepTime[1]--; }
        if (sleepTime[2] > 0) { sleepTime[2]--; }
        if (sleepTime[3] > 0) { sleepTime[3]--; }
    }

    RESTORESTATE;
    EA = 1;
    // reload 0x1C18 == 7192
    __asm
        MOV TH0, 0x1C
        MOV TL0, 0x18
        reti
    __endasm;

}

unsigned char now(void) {
    // time unit 20ms
    return displayTime1*10 + displayTime0;
}