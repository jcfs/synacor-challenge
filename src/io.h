#ifndef IO_H
#define IO_H

#include <stdint.h>

// prints the char represented by the argument
void putchr(uint16_t);

// gets a char from stdin
char getchr();

// creates the thread responsible for the IO
void create_io_thread();
#endif
