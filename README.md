== Synacor Challenge ==
In this challenge, your job is to use this architecture spec to create a
virtual machine capable of running the included binary.  Along the way,
you will find codes; submit these to the challenge website to track
your progress.  Good luck!


== architecture ==
- three storage regions
  - memory with 15-bit address space storing 16-bit values
  - eight registers
  - an unbounded stack which holds individual 16-bit values
- all numbers are unsigned integers 0..32767 (15-bit)
- all math is modulo 32768; 32758 + 15 => 5

== binary format ==
- each number is stored as a 16-bit little-endian pair (low byte, high byte)
- numbers 0..32767 mean a literal value
- numbers 32768..32775 instead mean registers 0..7
- numbers 32776..65535 are invalid
- programs are loaded into memory starting at address 0
- address 0 is the first 16-bit value, address 1 is the second 16-bit value, etc

== execution ==
- After an operation is executed, the next instruction to read is immediately after the last argument of the current operation.  If a jump was performed, the next operation is instead the exact destination of the jump.
- Encountering a register as an operation argument should be taken as reading from the register or setting into the register as appropriate.

== hints ==
- Start with operations 0, 19, and 21.
- Here's a code for the challenge website: NRsyGxrDOwMn
- The program "9,32768,32769,4,19,32768" occupies six memory addresses and should:
  - Store into register 0 the sum of 4 and the value contained in register 1.
  - Output to the terminal the character with the ascii code contained in register 0.

== opcode listing ==
- halt: 0
  stop execution and terminate the program
- set: 1 a b
  set register < A > to the value of < B >
- push: 2 a
  push < A > onto the stack
- pop: 3 a
  remove the top element from the stack and write it into < A >; empty stack = error
- eq: 4 a b c
  set < A > to 1 if < B > is equal to <c>; set it to 0 otherwise
- gt: 5 a b c
  set < A > to 1 if < B > is greater than <c>; set it to 0 otherwise
- jmp: 6 a
  jump to < A >
- jt: 7 a b
  if < A > is nonzero, jump to < B >
- jf: 8 a b
  if < A > is zero, jump to < B >
- add: 9 a b c
  assign into < A > the sum of < B > and <c> (modulo 32768)
- mult: 10 a b c
  store into < A > the product of < B > and <c> (modulo 32768)
- mod: 11 a b c
  store into < A > the remainder of < B > divided by <c>
- and: 12 a b c
  stores into < A > the bitwise and of < B > and <c>
- or: 13 a b c
  stores into < A > the bitwise or of < B > and <c>
- not: 14 a b
  stores 15-bit bitwise inverse of < B > in < A >
- rmem: 15 a b
  read memory at address < B > and write it to < A >
- wmem: 16 a b
  write the value from < B > into memory at address < A >
- call: 17 a
  write the address of the next instruction to the stack and jump to < A >
- ret: 18
  remove the top element from the stack and jump to it; empty stack = halt
- out: 19 a
  write the character represented by ascii code < A > to the terminal
- in: 20 a
  read a character from the terminal and write its ascii code to < A >; it can be assumed that once input starts, it will continue until a newline is encountered; this means that you can safely read whole lines from the keyboard and trust that they will be fully read
- noop: 21
  no operation