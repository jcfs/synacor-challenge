#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "vm.h"
#include "disassembler.h"

char ** d_program;

int print_instruction(char * buffer, uint16_t addr);
//
// External Functions
//

/*
 * Returns the current disassembled program
 * It may not have been initialized yet.
 */
char ** get_current_disassemble() {
  return d_program;
}

/*
 * Disassembles the whole memory. From addr 0 to addr <program_size-1>
 */
char ** disassemble() {
  return disassemble_range(0, program_size);
}

/*
 * Disassembles a range of memory addresses.
 */
char ** disassemble_range(uint16_t r_min, uint16_t r_max) {
  d_program = (char **) calloc(r_max - r_min, sizeof(char *));

  if (d_program == NULL) {
      perror("Insufficient memory\n");
      exit(1);
  }

  int i = r_min, addr;

  for(i = addr = r_min; i < r_max; i++) {
    d_program[i] = calloc(INST_MAX_SIZE, sizeof(char));

    if (d_program[i] == NULL) {
      perror("Insufficient memory\n");
      exit(1);
    }

    addr += print_instruction(d_program[i], addr);
  }

  return d_program;
}

/*
 * Frees the given disassemble memory block
 */
void free_disassemble(char ** d) {
  int i = 0;
  for(i = 0; i < program_size; i++) {
    free(d[i]);
  }

  free(d);
}

/**
 * Prints the current disassembled program in memory
 */
void disassemble_print_program() {
  int i = 0;

  if (d_program == NULL)
    disassemble();
  
  for(i = 0; i < program_size; i++)
    printf(d_program[i]);

}


/**
 * Updates an address in the current disassembled program 
 */
char ** update_dissassemble(uint16_t addr) {
  free(d_program[addr]);
  d_program[addr] = calloc(INST_MAX_SIZE, sizeof(char));
  print_instruction(d_program[addr], addr);
}

//
// Internal functions
//
int print_instruction(char * buffer, uint16_t addr) {
  int bw = 0;
  int opcode = mem[addr];


  bw += sprintf(buffer, "0x%04x: ", addr);
  
  if (opcode >= 22) {
    sprintf(buffer + bw, "0x%04x\n", mem[pc]);   
    return 1; 
  }

  bw += sprintf(buffer+bw, "%5s ", opcode_names[opcode]);

  uint8_t arg = opcode_pc[opcode];

  // print the first argument of the instruction
  if (arg >= 2) {
    if (IS_REG(mem[addr+1])) {
      bw += sprintf(buffer+bw, "R%d ", mem[addr+1]-32768);
    } else {
      if (opcode == 19 && mem[addr+1] != '\n')
        bw += sprintf(buffer+bw, "%c ", mem[addr+1]);
      else
        bw += sprintf(buffer+bw, "%x ", mem[addr+1]);
    }
  }


  // print the second argument of the instruction
  if (arg >= 3) {
    if (IS_REG(mem[addr+2])) {
      bw += sprintf(buffer+bw, "R%d ", mem[addr+2]-32768);
    } else {
      bw += sprintf(buffer+bw, "%x ", mem[addr+2]);
    }
  }

  // print the third argument of the instruction
  if (arg >= 4){
    if (IS_REG(mem[addr+3])) {
      bw += sprintf(buffer+bw, "R%d ", mem[addr+3]-32768);
    } else {
      bw += sprintf(buffer+bw, "%x ", mem[addr+3]);
    }
  }
  sprintf(buffer+bw, "\n");

  return opcode_pc[opcode];
}

