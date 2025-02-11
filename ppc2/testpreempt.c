#include <8051.h>
#include "preemptive.h" 

__data __at (0x20) static char shared_buffer;
__data __at (0x21) static char alphabet;
__data __at (0x22) static char producer_ready;

void producer(void) {
    
    while(1) {

        if (alphabet > 0x5A) {
            alphabet = 0x41;
        }

        while (producer_ready) { }

        EA = 0;
        shared_buffer = alphabet;
        alphabet++;
        producer_ready = 1;
        EA = 1;
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

        // wait producer
        while (!producer_ready) { }

        EA = 0;
        SBUF = shared_buffer; // write SBUF
        producer_ready = 0;
        EA = 1;

        while (!TI) { }
        TI = 0;
    }
}

void main(void) {

    producer_ready = 0;
    alphabet = 0x41;

    ThreadCreate(producer);  //Create Thread for producer;
    consumer();              //Call consumer;
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