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
uint16_t * sp = stack;
int program_size;

// functions definition
// stop execution and terminate the program
int halt(uint16_t a, uint16_t b, uint16_t c);
//set register <a> to the value of <b>
int set(uint16_t a, uint16_t b, uint16_t c);
//push <a> onto the stack
int push(uint16_t a, uint16_t b, uint16_t c);
//remove the top element from the stack and write it into <a>; empty stack = error
int pop(uint16_t a, uint16_t b, uint16_t c);
//set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
int eq(uint16_t a, uint16_t b, uint16_t c);
//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
int gt(uint16_t a, uint16_t b, uint16_t c);
//jump to <a>
int jmp(uint16_t a, uint16_t b, uint16_t c);
//if <a> is nonzero, jump to <b>
int jt(uint16_t a, uint16_t b, uint16_t c);
//if <a> is zero, jump to <b>
int jf(uint16_t a, uint16_t b, uint16_t c);
//assign into <a> the sum of <b> and <c> (modulo 32768)
int add(uint16_t a, uint16_t b, uint16_t c);
//store into <a> the product of <b> and <c> (modulo 32768)
int mult(uint16_t a, uint16_t b, uint16_t c);
//store into <a> the remainder of <b> divided by <c>
int mod(uint16_t a, uint16_t b, uint16_t c);
//stores into <a> the bitwise and of <b> and <c>
int and(uint16_t a, uint16_t b, uint16_t c);
//stores into <a> the bitwise or of <b> and <c>
int or(uint16_t a, uint16_t b, uint16_t c);
//stores 15-bit bitwise inverse of <b> in <a>
int not(uint16_t a, uint16_t b, uint16_t c);
//read memory at address <b> and write it to <a>
int rmem(uint16_t a, uint16_t b, uint16_t c);
//write the value from <b> into memory at address <a>
int wmem(uint16_t a, uint16_t b, uint16_t c);
//write the address of the next instruction to the stack and jump to <a>
int call(uint16_t a, uint16_t b, uint16_t c);
//remove the top element from the stack and jump to it; empty stack = halt
int ret(uint16_t a, uint16_t b, uint16_t c);
//write the character represented by ascii code <a> to the terminal
int out(uint16_t a, uint16_t b, uint16_t c);
//read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts
//it will continue until a newline is encountered; this means that you can safely read whole the keyboard and 
//trust that they will be fully read
int in(uint16_t a, uint16_t b, uint16_t c);
// no operation
int noop(uint16_t a, uint16_t b, uint16_t c);

char * opcode_names[22] = {
  "halt", "set", "push", "pop", "eq", 
  "gt", "jmp", "jt", "jf", "add", "mult", 
  "mod","and", "or", "not", "rmem", 
  "wmem", "call", "ret", "out", "in", "noop" 
};

int (*opcode_function[22])() = {
  (int *)&halt, (int *)&set, (int *)&push,
  (int *)&pop, (int *) &eq, (int *) &gt,
  (int *)&jmp, (int *) &jt, (int *) &jf,
  (int *)&add, (int *) &mult, (int *) &mod,(int*) &and,
  (int *)&or, (int *) &not, (int *) &rmem,
  (int *)&wmem, (int *) &call, (int *) &ret,
  (int *)&out, (int *) &in, (int *) &noop
};

uint8_t opcode_pc[22] = {
  1, 3, 2, 
  2, 4, 4,
  2, 3, 3,
  4, 4, 4,
  4, 4, 3,
  3, 3, 2,
  1, 2, 2, 1
};

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

  program_size = offset;
  printf("Loaded program with %d bytes\n", offset); 

  close(fd);
}

// runs the program. return 0 for successfull run
// and != 0 for error
int debug = 0;
int debug_s(int opcode) {

  printf("%5s ", opcode_names[opcode]);
  uint8_t arg = opcode_pc[opcode];

  if (arg>=2) {
    if (mem[pc+1] >= 32768) {
      printf("%c ", 'A' + mem[pc+1]-32768);
    } else {
      if (opcode == 19) 
        printf("%c ", mem[pc+1]);
      else 
        printf("%x ", mem[pc+1]);
    }
  }

  if (arg>=3) {
    if (mem[pc+2] >= 32768) {
      printf("%c ", 'A' + mem[pc+2]-32768);
    } else {
      printf("%x ", mem[pc+2]);
    }
  }

  if (arg >= 4){
    if (mem[pc+3] >= 32768) {
      printf("%c ", 'A' + mem[pc+3]-32768);
    } else {
      printf("%x ", mem[pc+3]);
    }
  }

  if (opcode == 17 || opcode == 18) printf("\n");

}

void disassemble() {
  printf("Dissassemble\n");
  while(pc <  program_size) {
    printf("0x%04x: ", pc);
    if (mem[pc] < 22) {
      debug_s(mem[pc]);
      pc += opcode_pc[mem[pc]];
    } else {
      pc++;
    }
    printf("\n");
  }
}

int run() {
  while(1) {
    uint16_t opcode = mem[pc];

    if (debug) {
      debug_s(opcode);
    }    

    pc += ((*opcode_function[opcode])(A,B,C) ? opcode_pc[opcode] : 0);
  }
}

int main(int argc, char **argv) {
  uint8_t run_p;
  uint8_t disas_p;

  if (argc != 3) {
    printf("usage: %s [option] <program_to_load>\nOptions:\n  -r: run porgram\n  -s: dissassemble program\n", argv[0]);
    exit(1);
  }

  load(argv[2]);

  if (!strncmp(argv[1], "-r", 2)) {
    run();
  } else if (!strncmp(argv[1], "-s", 2)) {
    disassemble();
  }
}
