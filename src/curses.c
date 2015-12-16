#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>
#include <pthread.h>

#include "vm.h"
#include "curses.h"
#include "disassembler.h"

// control flags
uint8_t curses_running, status_update, disassembler_update;

// window structs
WINDOW * vm_window;
WINDOW * disassembler_window;
WINDOW * status_window;

void init_curses();
void init_windows();
void update_status();
void update_disassembler();

// screen dimensions
int parent_x, parent_y;
int disassembler_program_row = 0;

int scroll_up() {
  disassembler_update = WINDOW_UPDATE;

  disassembler_program_row-=30;
  if (disassembler_program_row < 0)
    disassembler_program_row = 0;
}
int scroll_down() {
  disassembler_update = WINDOW_UPDATE;
  disassembler_program_row+=30;
}

// main function that actually runs the program and updates all the windows
int run_curses() {
  init_curses();
  init_windows();

  create_io_thread();
  disassemble();

  int flag = 0;

  disassembler_update = WINDOW_UPDATE;
  while(1) {
    uint16_t opcode = mem[pc];

    if (!flag && opcode == 20) { 
      status_update = WINDOW_UPDATE;
    }

    update_disassembler();
    update_status();

    // run program
    pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
  }

  endwin();
}

//
// Internal functions
//

void init_curses() {
  initscr();
  noecho();
  curs_set(FALSE);
  curses_running = 1;
}

// print borders to a screen
void draw_borders(WINDOW *screen) {
  int x, y, i;
  getmaxyx(screen, y, x);

  for (i = 0; i < (x); i++) {
    mvwprintw(screen, 0, i, "=");
    mvwprintw(screen, y - 1, i, "=");
  }

  for(i = 1; i < (y-7); i++) {
    mvwprintw(screen, i, x-34, "| ");
  }
}

// creates all the vm windows and set their basic parameters
// vm_window: window that gets the output of the program
// disassembler_window: window with the disassembled code
// status_window: window with the current vm status
void init_windows() {
  getmaxyx(stdscr, parent_y, parent_x);

  // windows creation
  vm_window = newwin(parent_y - 8, parent_x - 35, 1, 1);
  disassembler_window = newwin(parent_y - 8, 30, 1, parent_x-32);
  status_window = newwin(7, parent_x, parent_y-7,0);

  // draw the boarders
  draw_borders(stdscr);
  draw_borders(status_window);

  mvwprintw(stdscr, 0, parent_x/2-12, "  Synacor Challenge VM  ");
  scrollok(vm_window, true); 
  scrollok(disassembler_window, true); 
  scrollok(status_window, true); 

  wrefresh(stdscr);
  wrefresh(status_window);
}


// update status window with registers and pc infirmation
void update_status() {
  if (status_update & WINDOW_UPDATE) {
    status_update = 0;
    mvwprintw(status_window, 1, 1, "> Synacor Challenge VM v0.0.1-super-mega-alpha");
    mvwprintw(status_window, 2, 1, "> Program Size: %dB\tProgram Counter: 0x%x\t", program_size*sizeof(uint16_t), pc);
    mvwprintw(status_window, 3, 1, "> A: %5d\tB: %5d\tC: %5d\tD: %5d\t", reg[0], reg[1], reg[2], reg[3]);
    mvwprintw(status_window, 4, 1, "> E: %5d\tF: %5d\tG: %5d\tH: %5d\t", reg[4], reg[5], reg[6], reg[7]);
    wrefresh(status_window);
  }
}

uint8_t pc_is_inside = 0;
// updates the disassembler window if it must be updated
void update_disassembler() {
  int i = 0;

  disassembler_update |= pc_in_disassembler_window() ? WINDOW_UPDATE : NO_UPDATE;
  
  if (disassembler_update & WINDOW_UPDATE || pc_is_inside) {

    pc_is_inside = disassembler_update & WINDOW_UPDATE;
    disassembler_update &= NO_UPDATE;

    char ** d = get_current_disassemble();

    for(i = 0; i < parent_y - 9; i++) {
      if (pc_mapping[disassembler_program_row+i] == pc) {
        wattron(disassembler_window, A_REVERSE);
        mvwprintw(disassembler_window, i, 0, "%s\n", d[disassembler_program_row+i]);
        wattroff(disassembler_window, A_REVERSE);
      } else {
        mvwprintw(disassembler_window, i, 0, "%s\n", d[disassembler_program_row+i]);
      }
    }
    wrefresh(disassembler_window);
  }
}

// validates if the program counter is inside the disassembler window
// it is used when we need to know if we should update the disassembler window
int pc_in_disassembler_window() {
  int i = 0;

  for(i = 0; i< parent_y - 8; i++) {
    if (pc_mapping[disassembler_program_row+i] == pc)
      return TRUE;
  }

  return FALSE;
}



