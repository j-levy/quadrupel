
CC=gcc
CFLAGS = -g -Wall -lm
LDFLAGS= 
# LDFLAGS=-I/usr/include/SDL2 -D_REENTRANT -L/usr/lib64 -lSDL2 -Wl,--no-undefined -lm -ldl -lm -ldl -lpthread -lpthread -lrt

EXEC = ./pc-terminal
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)


all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)


%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC).o $(EXEC)

run: all
	$(EXEC)

	