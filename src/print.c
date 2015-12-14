#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>

#include "print.h"
#include "curses.h"

void print(uint16_t ch) {

  if (curses_running) {
    wprintw(vm_window, "%c", ch);
    wrefresh(vm_window);
  } else {
    putchar(ch);
  }

}
