LIBS  = -lncurses
CFLAGS = -Wall

OBJS = disassembler.o vm.o

all: vm

vm: disassembler.o vm.o
	gcc -o $@ $(OBJS) $(LIBS)

disassembler.o: disassembler.c disassembler.h
	gcc -c disassembler.c

vm.o: vm.c vm.h 
	gcc -c vm.c

clean:
	@rm *.o
	@rm vm
