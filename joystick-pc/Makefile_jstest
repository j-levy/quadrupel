
PRGM=joytest
FILE=jstest.c

all: compile
	./$(PRGM) --event /dev/input/js0

compile: $(FILE)
	gcc -o $(PRGM) $(FILE) `sdl2-config --cflags --libs`