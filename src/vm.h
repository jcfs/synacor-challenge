#ifndef VM_H
#define VM_H

#include <stdint.h>

#define MEM_SIZE      65536
#define STACK_SIZE    65536

#define IS_REG(x)     x >= 32768

#define REG(x)        (mem[x]-32768)
#define ARG(x)        (mem[x] <= 32767 ? mem[x] : reg[mem[x]-32768])
#define SET_REG(x, y) (reg[mem[x]-32768] = y)

#define A             ARG(pc+1)
#define B             ARG(pc+2)
#define C             ARG(pc+3)

extern uint16_t mem[MEM_SIZE];
extern uint16_t stack[];
extern uint16_t reg[8];
extern uint16_t pc;
extern uint16_t * sp;
extern uint16_t program_size;

extern char * opcode_names[];
extern uint8_t opcode_pc[];
extern int (*opcode_function[22])(uint16_t, uint16_t, uint16_t);
extern int run();

#endif
