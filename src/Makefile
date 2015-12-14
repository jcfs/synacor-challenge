LIBS  = -lncurses
CFLAGS = -Wall

OBJS = disassembler.o vm.o curses.o io.o main.o

all: svm

svm: main.o disassembler.o vm.o curses.o io.o
	gcc -o $@ $(OBJS) $(LIBS)

disassembler.o: disassembler.c disassembler.h
	gcc -c disassembler.c

curses.o: curses.c
	gcc -c curses.c

vm.o: vm.c vm.h
	gcc -c vm.c

io.o: io.c 
	gcc -c io.c

main.o: main.c
	gcc -c main.c

clean:
	@rm *.o
	@rm svm
