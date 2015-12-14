LIBS  = -lncurses
CFLAGS = -Wall

OBJS = disassembler.o vm.o curses.o print.o

all: svm

svm: disassembler.o vm.o curses.o print.o
	gcc -o $@ $(OBJS) $(LIBS)

disassembler.o: disassembler.c disassembler.h
	gcc -c disassembler.c

curses.o: curses.h curses.c
	gcc -c curses.c

vm.o: vm.c vm.h 
	gcc -c vm.c

print.o: print.c print.h
	gcc -c print.c

clean:
	@rm *.o
	@rm svm
