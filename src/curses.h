#ifndef CURSESS_H
#define CURSESS_H

extern uint8_t curses_running;
extern WINDOW * vm_window;
extern WINDOW * disassembler_window;


int run_curses();

#endif
