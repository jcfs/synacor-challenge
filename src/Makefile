LIBS  = -lncurses -lpthread 
CFLAGS = -Wall

OBJS = disassembler.o vm.o curses.o io.o main.o

all: svm

svm: main.o disassembler.o vm.o curses.o io.o
	gcc -o $@ $(OBJS) $(LIBS) $(CFLAGS)

disassembler.o: disassembler.c disassembler.h
	gcc -c disassembler.c $(CFLAGS)

curses.o: curses.c
	gcc -c curses.c $(CFLAGS)

vm.o: vm.c vm.h
	gcc -c vm.c $(CFLAGS)

io.o: io.c 
	gcc -c io.c $(CFLAGS)

main.o: main.c
	gcc -c main.c $(CFLAGS)

clean:
	@rm *.o
	@rm svm
