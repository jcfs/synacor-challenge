#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>

#include "vm.h"
#include "curses.h"

uint8_t curses_running = 0;
WINDOW * vm_window;
WINDOW * disassembler_window;

void draw_borders(WINDOW *);

void curse_init() {

}

void draw_windows() {

}

int run_curses() {
  int parent_x, parent_y;
  int score_size = 3;

  initscr();
  noecho();
  curs_set(FALSE);
  curses_running = 1;

  getmaxyx(stdscr, parent_y, parent_x);

  vm_window = newwin(parent_y - score_size-2, parent_x-2, 1, 1);

  draw_borders(stdscr);
  wrefresh(stdscr);

  int i;
  scrollok(vm_window, true); 

  while(1) {
    uint16_t opcode = mem[pc];

    pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
  }

  endwin();

}

//
// Internal functions
//

void draw_borders(WINDOW *screen) {
  int x, y, i;
  getmaxyx(screen, y, x);

  mvwprintw(screen, 0, 0, "+");

  mvwprintw(screen, y - 1, 0, "+");
  mvwprintw(screen, 0, x - 1, "+");
  mvwprintw(screen, y - 1, x - 1, "+");

  for (i = 1; i < (y - 1); i++) {
    mvwprintw(screen, i, 0, "|");
    mvwprintw(screen, i, x - 1, "|");
  }

  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, 0, i, "-");
    mvwprintw(screen, y - 1, i, "-");
  }


}

