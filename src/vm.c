#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vm.h"
#include "io.h"

uint16_t mem[MEM_SIZE];
uint16_t stack[100000000];
uint16_t reg[8];
uint16_t pc;
uint16_t * sp = stack;
uint16_t program_size;

// functions definition
// stop execution and terminate the program
static int halt(uint16_t a, uint16_t b, uint16_t c);
//set register <a> to the value of <b>
static int set(uint16_t a, uint16_t b, uint16_t c);
//push <a> onto the stack
static int push(uint16_t a, uint16_t b, uint16_t c);
//remove the top element from the stack and write it into <a>; empty stack = error
static int pop(uint16_t a, uint16_t b, uint16_t c);
//set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
static int eq(uint16_t a, uint16_t b, uint16_t c);
//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
static int gt(uint16_t a, uint16_t b, uint16_t c);
//jump to <a>
static int jmp(uint16_t a, uint16_t b, uint16_t c);
//if <a> is nonzero, jump to <b>
static int jt(uint16_t a, uint16_t b, uint16_t c);
//if <a> is zero, jump to <b>
static int jf(uint16_t a, uint16_t b, uint16_t c);
//assign into <a> the sum of <b> and <c> (modulo 32768)
static int add(uint16_t a, uint16_t b, uint16_t c);
//store into <a> the product of <b> and <c> (modulo 32768)
static int mult(uint16_t a, uint16_t b, uint16_t c);
//store into <a> the remainder of <b> divided by <c>
static int mod(uint16_t a, uint16_t b, uint16_t c);
//stores into <a> the bitwise and of <b> and <c>
static int and(uint16_t a, uint16_t b, uint16_t c);
//stores into <a> the bitwise or of <b> and <c>
static int or(uint16_t a, uint16_t b, uint16_t c);
//stores 15-bit bitwise inverse of <b> in <a>
static int not(uint16_t a, uint16_t b, uint16_t c);
//read memory at address <b> and write it to <a>
static int rmem(uint16_t a, uint16_t b, uint16_t c);
//write the value from <b> into memory at address <a>
static int wmem(uint16_t a, uint16_t b, uint16_t c);
//write the address of the next instruction to the stack and jump to <a>
static int call(uint16_t a, uint16_t b, uint16_t c);
//remove the top element from the stack and jump to it; empty stack = halt
static int ret(uint16_t a, uint16_t b, uint16_t c);
//write the character represented by ascii code <a> to the terminal
static int out(uint16_t a, uint16_t b, uint16_t c);
//read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts
//it will continue until a newline is encountered; this means that you can safely read whole the keyboard and 
//trust that they will be fully read
static int in(uint16_t a, uint16_t b, uint16_t c);
// no operation
static int noop(uint16_t a, uint16_t b, uint16_t c);



// opcode names order
char * opcode_names[22] = {
  "halt", "set", "push", "pop", "eq", 
  "gt", "jmp", "jt", "jf", "add", "mult", 
  "mod","and", "or", "not", "rmem", 
  "wmem", "call", "ret", "out", "in", "noop" 
};

// opcode function pointers
int (*opcode_function[22])(uint16_t, uint16_t, uint16_t) = {
  halt, set, push, pop, eq, gt, jmp, jt, jf,
  add, mult, mod, and, or, not, rmem, wmem, 
  call, ret, out, in, noop
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
static int halt(uint16_t a, uint16_t b, uint16_t c) {
  exit(0);
}

static int set(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b);
  return 1;
}

static int push(uint16_t a, uint16_t b, uint16_t c) {
  *sp++ = a;
  return 1;
}

static int pop(uint16_t a, uint16_t b, uint16_t c) {
  sp--;
  SET_REG(pc + 1, *sp);
  return 1;
}

static int eq(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b == c);
  return 1;
}

static int gt(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b > c);
  return 1;
}

static int jmp(uint16_t a, uint16_t b, uint16_t c) {
  pc = a;
  return 0;
}

static int jt(uint16_t a, uint16_t b, uint16_t c) {
  if (a) {
    pc = b;
    return 0;
  } else {
    return 1;
  }
}

static int jf(uint16_t a, uint16_t b, uint16_t c) {
  if (!a) {
    pc = b;
    return 0;
  } else {
    return 1;
  }
}

static int add(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b + c) % 32768);
  return 1;
}

static int mult(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b * c) % 32768);
  return 1;
}

static int mod(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b % c));
  return 1;
}

static int and(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b & c));
  return 1;
}

static int or(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b | c));
  return 1;
}

static int not(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b^0x7FFF);
  return 1;
}

static int rmem(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, mem[b]);
  return 1;
}

static int wmem(uint16_t a, uint16_t b, uint16_t c) {
  mem[a] = b;
  return 1;
}

static int call(uint16_t a, uint16_t b, uint16_t c) {
  *sp++ = pc + 2;
  pc = a;
  return 0;
}

static int ret(uint16_t a, uint16_t b, uint16_t c) {
  if (sp == stack) halt(a, b, c);
  sp--;
  pc = *sp;
  return 0;
}

static int out(uint16_t a, uint16_t b, uint16_t c) {
  putchr(a);
  return 1;
}

static int in(uint16_t a, uint16_t b, uint16_t c) {
  char ch = getchr();

  if (ch == 255) {
    return 0;
  } else {
    SET_REG(pc + 1, ch);
    return 1;
  }
}

static int noop(uint16_t a, uint16_t b, uint16_t c) {
  return 1;
}

// run the program loaded in memory
int run() {
  while(1) {
    uint16_t opcode = mem[pc];

    pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
  }
}

