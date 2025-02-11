#include <8051.h>
#include "preemptive.h" 

__data __at (0x20) static char alphabet;
__data __at (0x21) static char shared_buffer[3];
__data __at (0x25) static char empty;
__data __at (0x26) static char full;
__data __at (0x27) static char mutex;
__data __at (0x28) static char n; // initial value of semaphore
__data __at (0x29) static char produce_index;
__data __at (0x2A) static char consume_index;

// convert CNAME(s) to _s
#define CNAME(s) _##s

// convert __COUNTER__ to actual label
#define LABEL(label) label##$

// MOV CNAME(s) CNAME(n): using identifier in assembly
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

#define SemaphoreWaitBody(s, label) {\
    __asm                            \
        LABEL(label):                \
        MOV ACC, CNAME(s)            \
        JB ACC.7, LABEL(label)       \
        JZ LABEL(label)              \
        DEC CNAME(s)                 \
    __endasm;                        \
}

#define SemaphoreWait(s) {             \
    SemaphoreWaitBody(s, __COUNTER__)  \
}

void producer(void) {
    
    while(1) {
        if (alphabet > 0x5A) {
            alphabet = 0x41;
        }

        SemaphoreWait(empty);
        SemaphoreWait(mutex);
        EA = 0;

        // semaphore will do the number control
        // no need to worry about buffer spilling
        shared_buffer[produce_index] = alphabet;
        produce_index++;
        if (produce_index == 3) {
            produce_index = 0;
        }
        alphabet++;
        
        EA = 1;
        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
    }
}

void consumer(void) {

    // initialization
    TMOD |= 0x20; 
    TH1 = -6; 
    SCON = 0x50; 
    TR1 = 1;
    TI = 0;

    while(1) {
        SemaphoreWait(full);
        SemaphoreWait(mutex);
        EA = 0;

        SBUF = shared_buffer[consume_index]; // write SBUF
        consume_index++;
        if (consume_index == 3) {
            consume_index = 0;
        }
    
        EA = 1;
        SemaphoreSignal(mutex);
        SemaphoreSignal(empty);
        
        while (!TI) { }
        TI = 0;
    }
}

void main(void) {

    produce_index = 0;
    consume_index = 0;
    alphabet = 0x41;

    n = 0;
    SemaphoreCreate(full, n);
    n = 3;
    SemaphoreCreate(empty, n);
    n = 1;
    SemaphoreCreate(mutex, n);

    ThreadCreate(producer);
    consumer();
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