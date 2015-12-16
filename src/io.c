#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>

#include "io.h"
#include "curses.h"

pthread_t io_thread;

// io buffer
char vm_shared_io_buffer[64000];
uint16_t io_buffer_index;
pthread_mutex_t io_mutex;

// validates if the character is a valid character to pass to the 
// program loaded on the vm
int valid_program_char(char ch) {
  return ('a' <= ch && 'z' >= ch) || ch == '\r' || ch == '\n' || ch == ' ';
}

// function that handles IO
void * handle_io() {
  while(1) {
    char ch = getchar_unlocked();
    if (valid_program_char(ch)) {
      pthread_mutex_lock(&io_mutex);
      vm_shared_io_buffer[io_buffer_index++] = (ch == '\r' ? '\n' : ch); // stupid hack, dunno why it is needed yet
      pthread_mutex_unlock(&io_mutex);
    } else {
      if (ch == '-') {
        scroll_down();
      } else if (ch == '.') {
        scroll_up();
      }
    }
  }
}

// create thread that handles IO
// it is needed to split the IO to the VM from the IO to the program itself
void create_io_thread() {
  if(pthread_create(&io_thread, NULL, handle_io, NULL)) {
    perror("Error creating thread\n");
    exit(1);
  } 
}

// gets the char from stdin - if curses mode is active
// the char has to be read from the shared io buffer
char getchr() {
  char ch;

  // if using ncurses we used the shared io buffer 
  // to read the characters from - we need to syncronize it
  if (curses_running) {
    pthread_mutex_lock(&io_mutex);
    if (io_buffer_index > 0 && vm_shared_io_buffer[io_buffer_index - 1]) {
      ch = vm_shared_io_buffer[--io_buffer_index];
      // this is here to release the lock as soon as possible
      pthread_mutex_unlock(&io_mutex);
      wprintw(vm_window, "%c", ch);
      wrefresh(vm_window);
      return ch;
    } else {
      ch = 255;
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
  if (curses_running) {
    wprintw(vm_window, "%c", ch);
    wrefresh(vm_window);
  } else {
    putchar(ch);
  }
}
