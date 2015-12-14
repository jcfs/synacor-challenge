#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

// instruction string max size
#define INST_MAX_SIZE   128

// gets the current disassembled program
char ** get_current_disassemble();

// disassemble the whole memory
char ** disassemble();

// disassemble the range of memory addresses
char ** disassemble_range(uint16_t r_min, uint16_t r_max);

// print disassembled program
void disassemble_print_program();

// frees the disassemble structure pointed by argument
void free_disassemble(char ** program);

// update the disassemble at a specific memory address
char ** update_disassemble(uint16_t addr);



#endif
