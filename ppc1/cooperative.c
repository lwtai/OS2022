#include <8051.h>
#include "cooperative.h"

__data __at (0x30) static char zero_constant = (char)0x00;
__data __at (0x31) static char thread_bitmap;
__data __at (0x32) static char current_thread_id;
__data __at (0x33) static char saved_sp[MAXTHREADS];
__data __at (0x38) static char tmp_bitmap;
__data __at (0x39) static char tmp_sp;
__data __at (0x3A) static char tmp_psw = (char)0x00;
__data __at (0x3B) unsigned static char next_bit;


#define SAVESTATE {     \
        __asm           \
            PUSH ACC    \
            PUSH B      \
            PUSH DPL    \
            PUSH DPH    \
            PUSH PSW    \
        __endasm;       \
        saved_sp[current_thread_id] = SP; \
}   
 
#define RESTORESTATE {          \
        SP = saved_sp[current_thread_id];  \
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

    thread_bitmap = (char)0x00;
    saved_sp[0] = 0x3F;
    saved_sp[1] = 0x4F;
    saved_sp[2] = 0x5F;
    saved_sp[3] = 0x6F;
    current_thread_id = ThreadCreate(main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp){

    if(thread_bitmap == (char)0x0F){
        return -1;
    }   

    char bit_to_set = 0x00;
    ThreadID selected_thread_id = 0;

    // update bitmap
    bit_to_set = ((thread_bitmap+1) & (~thread_bitmap));
    thread_bitmap |= bit_to_set;

    // calculate thread ID
    while (bit_to_set >>= 1) {
        selected_thread_id++;
    }

    // calculate corresponding SP
    char start_sp = saved_sp[selected_thread_id];
    // char start_sp = (char)(((selected_thread_id | 0x04) << 4));

    // initial data structure for thread
    tmp_sp = SP; 
    SP = start_sp;

    // new psw value
    tmp_psw = (PSW & 0xE7);
    tmp_psw |= (selected_thread_id << 3);

    __asm   
        PUSH DPL             ;; low-byte  of fp parameter
        PUSH DPH             ;; high-byte of fp parameter
        PUSH _zero_constant  ;; ACC
        PUSH _zero_constant  ;; B 
        PUSH _zero_constant  ;; DPL
        PUSH _zero_constant  ;; DPH
        PUSH _tmp_psw        ;; PSW 
    __endasm;

    saved_sp[selected_thread_id] = SP;
    SP = tmp_sp;

    return selected_thread_id;
}


void ThreadYield(void) {

    SAVESTATE;

    next_bit = (char)0x01 << current_thread_id;
    char next_thread = current_thread_id;

    do {
        // RR
        if (next_bit == (char)0x08) {
            next_bit = 0x01;
            next_thread = 0;
        } else {
            next_bit <<= 1;
            next_thread++;
        }

        // check if active
        if (next_bit & thread_bitmap) {
            current_thread_id = next_thread;
            break;
        }
    }while(1);

    RESTORESTATE;
}


