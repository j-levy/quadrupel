
CC=gcc
CFLAGS = -g -Wall -lm
LDFLAGS= -lpthread
# LDFLAGS=-I/usr/include/SDL2 -D_REENTRANT -L/usr/lib64 -lSDL2 -Wl,--no-undefined -lm -ldl -lm -ldl -lpthread -lpthread -lrt

EXEC = ./pc-terminal
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)
LOGFILE = `date +%F_%Hh%M`.log
INCLUDES = pc_terminal.h


all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)


%.o: %.c $(INCLUDES)
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC).o $(EXEC)

run: all
	$(EXEC)

run-js1: all
	$(EXEC) /dev/input/js1

run-logger: all
	$(EXEC) >> $(LOGFILE) 2>&1

	