#include <8051.h>
#include "preemptive.h"

__data __at(0x20) static char numOfCar;
__data __at(0x21) static char infoOfCar[MAXTHREADS];
__data __at(0x25) static char threadId;
__data __at(0x26) static char tempVar;
__data __at(0x27) static char mutex;
__data __at(0x28) static char SemaphoreTempVar;

__data __at(0x4C) static char parkingMutex;
__data __at(0x4D) static char parkingControl;
__data __at(0x4E) static char parkingSpot[2];
__data __at(0x5A) static char tempVar2;
__data __at(0x5B) static char tempVar3;
__data __at(0x5C) static char infoOfSpot[MAXTHREADS];
__data __at(0x6A) static char tempVar4;
__data __at(0x6B) static char timeUnit;
__data __at(0x6C) static char infoOfTimeUnit[MAXTHREADS];
__data __at(0x7B) static char biggerTimeUnit;
__data __at(0x7C) static char infoOfBiggerTimeUnit[MAXTHREADS];

#define CNAME(s) _##s
#define LABEL(label) label##$
#define SemaphoreCreate(s, n) { \
    __asm                       \
        CLR 0xAF                \
        MOV CNAME(s), CNAME(n)  \
        SETB 0xAF               \
    __endasm;                   \
}
#define SemaphoreSignal(s) {    \
    __asm                       \
        INC CNAME(s)            \
    __endasm;                   \
}
#define SemaphoreWaitBody(s, label) { \
    __asm                             \
        DEC CNAME(s)                  \
        LABEL(label):                 \
        MOV ACC, CNAME(s)             \
        JB ACC.7, LABEL(label)        \
    __endasm;                         \
}
#define SemaphoreWait(s) {             \
    SemaphoreWaitBody(s, __COUNTER__)  \
}


void main(void) {

    // timer-1 setup for UART
    TMOD |= 0x20;
    TH1 = -6;
    SCON = 0x50;
    TR1 = 1;
    TI = 0;

    numOfCar = 0;
    parkingSpot[0] = 0x0F;
    parkingSpot[1] = 0x0F;
    SemaphoreTempVar = 1; SemaphoreCreate(mutex, SemaphoreTempVar);
    SemaphoreTempVar = 2; SemaphoreCreate(parkingControl, SemaphoreTempVar);
    SemaphoreTempVar = 1; SemaphoreCreate(parkingMutex, SemaphoreTempVar);
    ThreadCreate(CarParking);
    ThreadCreate(CarParking);
    ThreadCreate(CarParking);
    ThreadCreate(CarParking);
    ThreadCreate(CarParking);
    ThreadExit();
}

void CarParking(void) {

    // document corresponding thread to numOfCar
    // mutex protect numOfCar and infoOfCar
    SemaphoreWait(mutex);
    EA = 0;

    __asm
        MOV 0x25, 0x31  ;; find thread id
    __endasm;
    infoOfCar[threadId] = numOfCar;
    numOfCar++;

    EA = 1;
    SemaphoreSignal(mutex);

    // compete for spot
    // parkingMutex protect parkingSpot and infoOfSpot
    SemaphoreWait(parkingControl);
    SemaphoreWait(parkingMutex);
    EA = 0;

    __asm
        MOV 0x25, 0x31  ;; find thread id
    __endasm;
    if (parkingSpot[0] == 0x0F) {
        parkingSpot[0] = 0xFF;
        infoOfSpot[threadId] = 0;
    }
    else if (parkingSpot[1] == 0x0F) {
        parkingSpot[1] = 0xFF;
        infoOfSpot[threadId] = 1;
    }
    __asm
        MOV 0x6B, 0x2D  ;; find timeUnit
        MOV 0x7B, 0x4B  ;; find biggerTimeUnit
    __endasm;
    infoOfTimeUnit[threadId] = timeUnit;
    infoOfBiggerTimeUnit[threadId] = biggerTimeUnit;

    EA = 1;
    SemaphoreSignal(parkingMutex)

    // print information
    EA = 0;

    __asm
        MOV 0x25, 0x31  ;; find thread id
    __endasm;
    tempVar = infoOfCar[threadId];
    tempVar2 = infoOfSpot[threadId];
    tempVar3 = infoOfBiggerTimeUnit[threadId];
    tempVar4 = infoOfTimeUnit[threadId];

    SBUF = 'C'; while (!TI); TI = 0;
    SBUF = 'a'; while (!TI); TI = 0;
    SBUF = 'r'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = tempVar+48; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 'g'; while (!TI); TI = 0;
    SBUF = 'o'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 's'; while (!TI); TI = 0;
    SBUF = 'p'; while (!TI); TI = 0;
    SBUF = 'o'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = tempVar2+48; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 'a'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = tempVar3+48; while (!TI); TI = 0;
    SBUF = tempVar4+48; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 'u'; while (!TI); TI = 0;
    SBUF = 'n'; while (!TI); TI = 0;
    SBUF = 'i'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = '.'; while (!TI); TI = 0;
    SBUF = 10; while (!TI); TI = 0;

    EA = 1;

    delay(9);

    // leave the spot
    SemaphoreWait(parkingMutex);
    EA = 0;

    __asm
        MOV 0x25, 0x31  ;; find thread id
        MOV 0x6B, 0x2D  ;; find timeUnit
        MOV 0x7B, 0x4B  ;; find biggerTimeUnit
    __endasm;
    infoOfTimeUnit[threadId] = timeUnit;
    infoOfBiggerTimeUnit[threadId] = biggerTimeUnit;
    char temp = infoOfSpot[threadId];
    parkingSpot[temp] = 0x0F; // mark spot as free

    EA = 1;
    SemaphoreSignal(parkingMutex);
    SemaphoreSignal(parkingControl);

    // print information
    EA = 0;

    __asm
        MOV 0x25, 0x31  ;; find thread id
    __endasm;
    tempVar = infoOfCar[threadId];
    tempVar2 = infoOfSpot[threadId];
    tempVar3 = infoOfBiggerTimeUnit[threadId];
    tempVar4 = infoOfTimeUnit[threadId];

    SBUF = 'C'; while (!TI); TI = 0;
    SBUF = 'a'; while (!TI); TI = 0;
    SBUF = 'r'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = tempVar+48; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 'l'; while (!TI); TI = 0;
    SBUF = 'e'; while (!TI); TI = 0;
    SBUF = 'f'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 's'; while (!TI); TI = 0;
    SBUF = 'p'; while (!TI); TI = 0;
    SBUF = 'o'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = tempVar2+48; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 'a'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = tempVar3+48; while (!TI); TI = 0;
    SBUF = tempVar4+48; while (!TI); TI = 0;
    SBUF = ' '; while (!TI); TI = 0;
    SBUF = 'u'; while (!TI); TI = 0;
    SBUF = 'n'; while (!TI); TI = 0;
    SBUF = 'i'; while (!TI); TI = 0;
    SBUF = 't'; while (!TI); TI = 0;
    SBUF = '.'; while (!TI); TI = 0;
    SBUF = 10; while (!TI); TI = 0;

    EA = 1;

    ThreadExit();
}

void _sdcc_gsinit_startup(void) {
    __asm
        ljmp  _Bootstrap
    __endasm;
}

void _mcs51_genRAMCLEAR(void) { }
void _mcs51_genXINIT(void) { }
void _mcs51_genXRAMCLEAR(void) { }

// ISR
void timer0_ISR(void) __interrupt(1) {
    __asm
        ljmp _myTimer0Handler
    __endasm;
}