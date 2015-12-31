#ifndef CURSESS_H
#define CURSESS_H

#include <ncurses.h>

#define NO_UPDATE     0
#define WINDOW_UPDATE 1

#define CURSES_MODE_ACTIVE  1
#define TRACE_MODE_ACTIVE   1<<1
#define SET_BP_ACTIVE       1<<2
#define SET_VALUE_ACTIVE    1<<3

extern WINDOW * vm_window;
extern WINDOW * status_window;
extern WINDOW * disassembler_window;

// main function to run the vm with curses mode on
int run_curses();

// thread functions

uint32_t get_curses_mode(void);

void run_disassembler(void);
void dump_disassembler(void);
void enable_trace(void);
void next_step(void);
void enable_set_breakpoint(void);
void enable_set_value(void);
void disable_set_breakpoint(void);
void set_breakpoint(uint16_t);
#endif
