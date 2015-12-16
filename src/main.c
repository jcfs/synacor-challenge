#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h>

#include "io.h"
#include "vm.h"
#include "curses.h"
#include "disassembler.h"

void load(char * file) {
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
  printf("Loaded program with %d bytes\n", offset*sizeof(uint16_t));

  close(fd);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("usage: %s [option] <program_to_load>\nOptions:\n  -r: run porgram\n  -s: dissassemble program\n", argv[0]);
    exit(1);
  }

  load(argv[2]);

  if (!strncmp(argv[1], "-r", 2)) {
    run();
  } else  if (!strncmp(argv[1], "-c", 2)) {
    create_io_thread();
    run_curses();
  }  else if (!strncmp(argv[1], "-s", 2)) {
    disassemble_print_program();
  }

  return 0;
}

