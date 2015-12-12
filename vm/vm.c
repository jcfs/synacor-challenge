#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_CHUNK 65536
#define STACK_CHUNK 65536

#define REG(x) (mem[x]-32768)
#define ARG(x) (mem[x] <= 32767 ? mem[x] : reg[mem[x]-32768])
#define SET_REG(x, y) (reg[mem[x]-32768] = y)

#define A ARG(pc+1)
#define B ARG(pc+2)
#define C ARG(pc+3)

uint16_t mem[65536];
uint16_t stack[65536];
uint16_t reg[8];
uint16_t pc;
int sp = 0;

// functions definition
// stop execution and terminate the program
void halt(uint16_t a, uint16_t b, uint16_t c);
//set register <a> to the value of <b>
void set(uint16_t a, uint16_t b, uint16_t c);
//push <a> onto the stack
void push(uint16_t a, uint16_t b, uint16_t c);
//remove the top element from the stack and write it into <a>; empty stack = error
void pop(uint16_t a, uint16_t b, uint16_t c);
//set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
void eq(uint16_t a, uint16_t b, uint16_t c);
//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
void gt(uint16_t a, uint16_t b, uint16_t c);
//jump to <a>
void jmp(uint16_t a, uint16_t b, uint16_t c);
//if <a> is nonzero, jump to <b>
void jt(uint16_t a, uint16_t b, uint16_t c);
//if <a> is zero, jump to <b>
void jf(uint16_t a, uint16_t b, uint16_t c);
//assign into <a> the sum of <b> and <c> (modulo 32768)
void add(uint16_t a, uint16_t b, uint16_t c);
//store into <a> the product of <b> and <c> (modulo 32768)
void mult(uint16_t a, uint16_t b, uint16_t c);
//store into <a> the remainder of <b> divided by <c>
void mod(uint16_t a, uint16_t b, uint16_t c);
//stores into <a> the bitwise and of <b> and <c>
void and(uint16_t a, uint16_t b, uint16_t c);
//stores into <a> the bitwise or of <b> and <c>
void or(uint16_t a, uint16_t b, uint16_t c);
//stores 15-bit bitwise inverse of <b> in <a>
void not(uint16_t a, uint16_t b, uint16_t c);
//read memory at address <b> and write it to <a>
void rmem(uint16_t a, uint16_t b, uint16_t c);
//write the value from <b> into memory at address <a>
void wmem(uint16_t a, uint16_t b, uint16_t c);
//write the address of the next instruction to the stack and jump to <a>
void call(uint16_t a, uint16_t b, uint16_t c);
//remove the top element from the stack and jump to it; empty stack = halt
void ret(uint16_t a, uint16_t b, uint16_t c);
//write the character represented by ascii code <a> to the terminal
void out(uint16_t a, uint16_t b, uint16_t c);
//read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts
//it will continue until a newline is encountered; this means that you can safely read whole the keyboard and 
//trust that they will be fully read
void in(uint16_t a, uint16_t b, uint16_t c);
// no operation
void noop(uint16_t a, uint16_t b, uint16_t c);

void (*opcode_function[22])() = {
  &halt, &set, &push,
  (int *)&pop, (int *) &eq, (int *) &gt,
  (int *)&jmp, (int *) &jt, (int *) &jf,
  (int *)&add, (int *) &mult, (int *) &mod,(int*) &and,
  (int *)&or, (int *) &not, (int *) &rmem,
  (int *)&wmem, (int *) &call, (int *) &ret,
  (int *)&out, (int *) &in, (int *) &noop
};

void halt(uint16_t a, uint16_t b, uint16_t c) {
  exit(0);
}

void set(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b);
  pc += 3;
}

void push(uint16_t a, uint16_t b, uint16_t c) {
  stack[sp++] = a;
  pc += 2;
}

void pop(uint16_t a, uint16_t b, uint16_t c) {
  sp--;
  SET_REG(pc + 1, stack[sp]);
  pc += 2;
}

void eq(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b == c);
  pc += 4;
}

void gt(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b > c);
  pc += 4;
}

void jmp(uint16_t a, uint16_t b, uint16_t c) {
  pc = a;
}

void jt(uint16_t a, uint16_t b, uint16_t c) {
  pc = a ? b : pc + 3;
}

void jf(uint16_t a, uint16_t b, uint16_t c) {
  pc = !a ? b : pc + 3;
}

void add(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b + c) % 32768);
  pc += 4;
}

void mult(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b * c) % 32768);
  pc += 4;
}

void mod(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b % c));
  pc += 4;
}

void and(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b & c));
  pc += 4;
}

void or(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, (b | c));
  pc += 4;
}

void not(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, b^0x7FFF);
  pc += 3;
}

void rmem(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, mem[b]);
  pc += 3;
}

void wmem(uint16_t a, uint16_t b, uint16_t c) {
  mem[a] = b;
  pc += 3;
}

void call(uint16_t a, uint16_t b, uint16_t c) {
  stack[sp++] = pc + 2;
  pc = a;
}

void ret(uint16_t a, uint16_t b, uint16_t c) {
  if (!sp) halt(a, b, c);
  sp--;
  pc = stack[sp];
}

void out(uint16_t a, uint16_t b, uint16_t c) {
  putchar(a);
  pc += 2;
}

void in(uint16_t a, uint16_t b, uint16_t c) {
  SET_REG(pc + 1, getchar());
  pc += 2;
}

void noop(uint16_t a, uint16_t b, uint16_t c) {
  pc++;
}



// load the program to the vm memory
void load(char * file) {
  char ch;
  int fd = open(file, O_RDONLY);

  if (fd == -1) {
    printf("Error loading program [%s]\n", file);
    exit(1);
  }

  ssize_t br, offset = 0;
  while((br = read(fd, mem + offset, sizeof(uint16_t))) > 0) {
    offset++;
  }

  printf("Loaded program with %d bytes\n", offset); 

  close(fd);
}

// runs the program. return 0 for successfull run
// and != 0 for error
int run() {
  while(1) {
    uint16_t opcode = mem[pc];
    (*opcode_function[opcode])(A,B,C);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s <program_to_load>\n", argv[0]);
    exit(1);
  }

  //load the program
  load(argv[1]);

  //run the program
  run();
}
