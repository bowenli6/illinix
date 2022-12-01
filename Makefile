SRC_DIR = src
LIB_DIR = lib
ELF_DIR = elf

CFLAGS += -g -Wall -nostdlib -ffreestanding -Iinclude
LDFLAGS += -g -nostdlib -ffreestanding
CC = gcc

LIB = stdio.o stdlib.o string.o unistd.o syscall.o

ALL: 

%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: $(LIB_DIR)/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.exe: %.o stdio.o stdlib.o unistd.o string.o main.o 
	$(CC) $(LDFLAGS) -o $@ $^

%: %.exe
	./elfconvert $<
	mv $<.converted bin/$@

clean::
	rm -f *~ *.o

clear: clean
	rm -f *.converted
	rm -f *.exe
	rm -f to_fsdir/*
