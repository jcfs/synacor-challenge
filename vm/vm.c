#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h>

#include "vm.h"
#include "disassembler.h"

#define REG(x)        (mem[x]-32768)
#define ARG(x)        (mem[x] <= 32767 ? mem[x] : reg[mem[x]-32768])
#define SET_REG(x, y) (reg[mem[x]-32768] = y)

#define A             ARG(pc+1)
#define B             ARG(pc+2)
#define C             ARG(pc+3)

uint16_t mem[MEM_SIZE];
uint16_t stack[STACK_SIZE];
uint16_t reg[8];
uint16_t pc;
uint16_t * sp = stack;
uint16_t program_size;

// opcode names order
char * opcode_names[22] = {
  "halt", "set", "push", "pop", "eq", 
  "gt", "jmp", "jt", "jf", "add", "mult", 
  "mod","and", "or", "not", "rmem", 
  "wmem", "call", "ret", "out", "in", "noop" 
};

// opcode function pointers
int (*opcode_function[22])() = {
  (int *)&halt, (int *)&set, (int *)&push,
  (int *)&pop, (int *) &eq, (int *) &gt,
  (int *)&jmp, (int *) &jt, (int *) &jf,
  (int *)&add, (int *) &mult, (int *) &mod,(int*) &and,
  (int *)&or, (int *) &not, (int *) &rmem,
  (int *)&wmem, (int *) &call, (int *) &ret,
  (int *)&out, (int *) &in, (int *) &noop
};

// opcode pc increase array
uint8_t opcode_pc[22] = {
  1, 3, 2, 
  2, 4, 4,
  2, 3, 3,
  4, 4, 4,
  4, 4, 3,
  3, 3, 2,
  1, 2, 2, 1
};

// opcode function implementations
int halt(uint16_t a, uint16_t b, uint16_t c) {
  exit(0);
}

int set(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b);
  return 1;
}

int push(uint16_t a, uint16_t b, uint16_t c) {
  *sp++ = a;
  return 1;
}

int pop(uint16_t a, uint16_t b, uint16_t c) {
  sp--;
  SET_REG(pc + 1, *sp);
  return 1;
}

int eq(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b == c);
  return 1;
}

int gt(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b > c);
  return 1;
}

int jmp(uint16_t a, uint16_t b, uint16_t c) {
  pc = a;
  return 0;
}

int jt(uint16_t a, uint16_t b, uint16_t c) {
  if (a) {
    pc = b;
    return 0;
  } else {
    return 1;
  }
}

int jf(uint16_t a, uint16_t b, uint16_t c) {
  if (!a) {
    pc = b;
    return 0;
  } else {
    return 1;
  }
}

int add(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b + c) % 32768);
  return 1;
}

int mult(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b * c) % 32768);
  return 1;
}

int mod(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b % c));
  return 1;
}

int and(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b & c));
  return 1;
}

int or(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b | c));
  return 1;
}

int not(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b^0x7FFF);
  return 1;
}

int rmem(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, mem[b]);
  return 1;
}

int wmem(uint16_t a, uint16_t b, uint16_t c) {
  mem[a] = b;
  return 1;
}

int call(uint16_t a, uint16_t b, uint16_t c) {
  *sp++ = pc + 2;
  pc = a;
  return 0;
}

int ret(uint16_t a, uint16_t b, uint16_t c) {
  if (sp == stack) halt(a, b, c);
  sp--;
  pc = *sp;
  return 0;
}

int out(uint16_t a, uint16_t b, uint16_t c) {
  putchar(a);
  return 1;
}
int in(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, getchar());
  return 1;
}

int noop(uint16_t a, uint16_t b, uint16_t c) {
  return 1;
}

// load the program to the vm memory
void load(char * file) {
  int fd = open(file, O_RDONLY);

  if (fd == -1) {
    printf("Error loading program [%s]\n", file);
    exit(1);
  }

  ssize_t br, offset = 0;
  while((br = read(fd, mem + offset, sizeof(uint16_t))) > 0) {
    offset++;
  }

  program_size = offset;
  printf("Loaded program with %d bytes\n", offset); 

  close(fd);
}

// run the program loaded in memory
int run() {
  while(1) {
    uint16_t opcode = mem[pc];

    pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
  }
}

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
    mvwprintw(screen, i, x - 35, "|");
  }

  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, 0, i, "-");
    mvwprintw(screen, y - 1, i, "-");
  }


  mvwprintw(screen, 0, x - 35, "+");
  mvwprintw(screen, y - 1, x - 35, "+");
}

int run_curses() {
  int parent_x, parent_y;
  int score_size = 3;

  initscr();
  noecho();
  curs_set(FALSE);

  getmaxyx(stdscr, parent_y, parent_x);

  WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0);
  WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0);

  draw_borders(field);
  draw_borders(score);

  mvwprintw(field, 0, 0, "Field");
  mvwprintw(score, 0, 0, "Score");


  wrefresh(field);
  wrefresh(score);

  sleep(5);
  endwin();

}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("usage: %s [option] <program_to_load>\nOptions:\n  -r: run porgram\n  -s: dissassemble program\n", argv[0]);
    exit(1);
  }

  load(argv[2]);

  if (!strncmp(argv[1], "-r", 2)) {
    run();
  } else  if (!strncmp(argv[1], "-c", 2)) {
    run_curses();
  }  else if (!strncmp(argv[1], "-s", 2)) {
    disassemble_print_program();
  }
}
