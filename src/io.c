#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>

#include "io.h"
#include "curses.h"

#define MIXED_MODE      1
#define VM_MODE         2
#define PROG_MODE       4

pthread_t io_thread;

char input[64000];
uint16_t input_size, input_offset;

uint8_t io_mode;
// io buffer
char vm_shared_io_buffer[64000];
uint16_t io_buffer_index, io_buffer_ri;
pthread_mutex_t io_mutex;

// vm input function mapping


// validates if the character is a valid character to pass to the 
// program loaded on the vm
// should be properly implemented - not so hacky
int valid_program_char(char ch) {
  return ('a' <= ch && 'z' >= ch) || ch == '\r' || ch == '\n' || ch == ' ';
}

// handle input that should go to the vm instead of the program
// running
void handle_vm_io(char ch) {
  if (get_curses_mode() & SET_BP_ACTIVE) {
    char chr;
    int i = 0;
    char buffer[64];

    while((chr = getchar()) != '\r') {
      buffer[i++] = chr;
      mvwprintw(status_window, 5, 30+i, "%c", chr);
      wrefresh(status_window);
    }
    buffer[i] = 0;
    set_breakpoint(strtol(buffer, NULL, 16));
    disable_set_breakpoint();
  } else 

  // ALT mode on - we need to read one more character
  if (ch == 27) {
    ch = getchar();
    if (ch == 'd') run_disassembler();
    if (ch == 'x') dump_disassembler();
    if (ch == 'a') enable_trace();
    if (ch == 'e') next_step();
    if (ch == 'q') enable_set_breakpoint();
    if (ch == 'w') enable_set_value();
  }
}

// function that handles IO
void * handle_io() {
  while(1) {
    char ch;
    if (input_offset < input_size) {
      ch = input[input_offset++];
    } else {
      ch = getchar_unlocked();
    }
    if (valid_program_char(ch)) {
      pthread_mutex_lock(&io_mutex);
      vm_shared_io_buffer[io_buffer_index++] = ((ch == '\r' || ch == '\n') ? '\n' : ch); // stupid hack, dunno why it is needed yet

      if (io_buffer_index > 63999) {
        io_buffer_index = 0;
      }

      pthread_mutex_unlock(&io_mutex);
    } else {
      handle_vm_io(ch);
    }
  }
}

// create thread that handles IO
// it is needed to split the IO to the VM from the IO to the program itself
void create_io_thread() {
  if(pthread_create(&io_thread, NULL, handle_io, NULL)) {
    perror("Error creating thread\n");
    exit(1);
  } else {
    printf("IO Thread created successfuly\n");
  }
}

// gets the char from stdin - if curses mode is active
// the char has to be read from the shared io buffer
char getchr() {
  char ch;

  // if using ncurses we used the shared io buffer 
  // to read the characters from - we need to syncronize it
  if (get_curses_mode() & CURSES_MODE_ACTIVE) {
    pthread_mutex_lock(&io_mutex);
    if (io_buffer_ri < io_buffer_index || (io_buffer_ri && !io_buffer_index)) {
      ch = vm_shared_io_buffer[io_buffer_ri++];

      if (io_buffer_ri > 63999)
        io_buffer_ri = 0;
      
      // this is here to release the lock as soon as possible
      wprintw(vm_window, "%c", ch);
      wrefresh(vm_window);
      pthread_mutex_unlock(&io_mutex);
      return ch;
    } else {
      ch = -1;
    }

    pthread_mutex_unlock(&io_mutex);
    return ch;
  } else {
    return getchar();
  }
}

//writes the char to the screen - if curses mode is active
//the char is written to the vm window
void putchr(uint16_t ch) {
  if (get_curses_mode() & CURSES_MODE_ACTIVE) {
    wprintw(vm_window, "%c", ch);
    wrefresh(vm_window);
  } else {
    putchar(ch);
  }
}
