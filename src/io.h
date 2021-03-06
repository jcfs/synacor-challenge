#ifndef IO_H
#define IO_H

#include <stdint.h>

extern char input[64000];
extern uint16_t input_size;

// prints the char represented by the argument
void putchr(uint16_t);

// gets a char from stdin
char getchr();

// creates the thread responsible for the IO
void create_io_thread();
#endif
