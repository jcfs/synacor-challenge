#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>

#include "vm.h"
#include "curses.h"
#include "disassembler.h"

// control flags
static uint8_t status_update, disassembler_update;
static uint32_t running_mode;

// breakpoints array with a maximum of 64 breakpoints
uint16_t breakpoints[64] = { -1 };

// window structs
WINDOW * vm_window;
WINDOW * disassembler_window;
WINDOW * status_window;

static void init_curses();
static void init_windows();
static void print_labels();
static void update_status();
static void update_disassembler();

char should_run = 0;
char sbs_mode = 0;

// functions that need syncronization
// They are used by another thread
uint32_t get_curses_mode() {
  return running_mode;
}

// enables/disable trace mode, the disassembler 
// window should follow pc
void enable_trace() {
  if (running_mode & TRACE_MODE_ACTIVE) {
    running_mode = running_mode & ~(TRACE_MODE_ACTIVE&0xFFFFFFFF);
  } else {
    running_mode |= TRACE_MODE_ACTIVE;
  }
}

// re-run the disassembler to get fresh new disassembled 
// code with the current memory state
void run_disassembler() {
  disassemble();
}

void dump_disassembler() {

}

void set_breakpoint(uint16_t address) {
  int i = 0;

  printf("BREAKPOINT AT %x\n", address);
  for(i = 0; i < 64; i++) {
    if (breakpoints[i] != -1)
      breakpoints[i] = address;
  }
}

void next_step() {
  should_run = 1;
  
  reg[7] = 0x6486;
  mem[0x156d]=6;
  mem[0x1571]=0x15;
  mem[0x1572]=0x15;
}

// enables and disables the set register value mode
void enable_set_value() {
  running_mode |= SET_VALUE_ACTIVE;
}

void disable_set_value() {
  running_mode &= ~(SET_VALUE_ACTIVE&0xFFFFFFFF);
}

// enables and disables set breakpoint value mode
void enable_set_breakpoint() {
  running_mode |= SET_BP_ACTIVE;
}

void disable_set_breakpoint() {
  running_mode &= ~(SET_BP_ACTIVE&0xFFFFFFFF);
}

// screen dimensions
int parent_x, parent_y;
int disassembler_program_row = 0;

// main function that actually runs the program and updates all the windows
int run_curses() {
  int i = 0;
  init_curses();
  init_windows();

  disassemble();

  disassembler_update = WINDOW_UPDATE;


  for(i = 0; i < 64; i++) breakpoints[i] = -1;
  while(1) {

    uint16_t opcode = mem[pc];

    status_update = WINDOW_UPDATE;

    update_disassembler();
    update_status();

    for(i = 0; i < 64; i++) {
      if (breakpoints[i] == pc)
        sbs_mode = 1;
    }

    if (sbs_mode && should_run) {
      should_run = 0;
      pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
    } 
    
    if (!sbs_mode) {
      pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
    }
  }

  endwin();
}

//
// Internal functions
//

static void init_curses() {
  initscr();
  noecho();
  curs_set(FALSE);
  running_mode |= CURSES_MODE_ACTIVE;
}

// print borders to a screen
static void draw_borders(WINDOW *screen) {
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
static void init_windows() {
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
  print_labels();
}

static void print_pair(WINDOW * window, char * label, char * value, int x, int y) {
  wattron(window, A_REVERSE); 
  mvwprintw(window, y, x, label);
  wattroff(window, A_REVERSE);
  mvwprintw(window, y, x+strlen(label), value);
}

static void print_labels() {
  mvwprintw(status_window, 1, 1, "> Synacor Challenge VM v0.0.1-super-mega-alpha");
  print_pair(status_window, "^D", " Disassemble", parent_x - 33, 1);
  print_pair(status_window, "^X", " Dump", parent_x - 18, 1);
  print_pair(status_window, "^A", " Trace", parent_x - 33, 2);
  print_pair(status_window, "^E", " Step", parent_x - 18, 2);
  print_pair(status_window, "^Q", " Breakpoint", parent_x - 33, 3);
  print_pair(status_window, "^W", " Set Value", parent_x - 18, 3);
  wrefresh(status_window);
}

// update status window with registers and pc infirmation
static void update_status() {
  if (running_mode & TRACE_MODE_ACTIVE) {
    mvwprintw(status_window, 2, 1, "> Program Size: %dB\t\tProgram Counter: 0x%x\t", program_size*sizeof(uint16_t), pc);
    mvwprintw(status_window, 3, 1, "> A: %5d\tB: %5d\tC: %5d\tD: %5d\t", reg[0], reg[1], reg[2], reg[3]);
    mvwprintw(status_window, 4, 1, "> E: %5d\tF: %5d\tG: %5d\tH: %5d\t", reg[4], reg[5], reg[6], reg[7]);
    wrefresh(status_window);
  }
}


// validates if the program counter is inside the disassembler window
// it is used when we need to know if we should update the disassembler window
// NOT CURRENTLY USED

static uint8_t pc_is_inside = 0;
// updates the disassembler window if it must be updated
static void update_disassembler() {
  int i = 0;

  if (running_mode & TRACE_MODE_ACTIVE) {
    for(i = 0; i < program_size; i++) {
      if (pc_mapping[i] == pc) {
        disassembler_update |= WINDOW_UPDATE;
        disassembler_program_row = i;
      }
    }
  }

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
