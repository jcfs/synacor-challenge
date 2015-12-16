#ifndef CURSESS_H
#define CURSESS_H

#include <ncurses.h>

#define NO_UPDATE     0
#define WINDOW_UPDATE 1

extern uint8_t curses_running;
extern WINDOW * vm_window;
extern WINDOW * disassembler_window;


int run_curses();
void scroll_down();
void scroll_up();
#endif
