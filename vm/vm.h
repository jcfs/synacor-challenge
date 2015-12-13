#ifndef VM_H
#define VM_H

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


#endif
