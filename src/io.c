#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>

#include "io.h"
#include "curses.h"

char getchr() {
  if (curses_running) {
    return wgetch(vm_window);
  } else {
    return getchar();
  }
}

void print(uint16_t ch) {

  if (curses_running) {
    wprintw(vm_window, "%c", ch);
    wrefresh(vm_window);
  } else {
    putchar(ch);
  }

}
