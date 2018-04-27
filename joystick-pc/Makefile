
PRGM=joytest
FILE=js_sdl.c

all: compile
	./$(PRGM)

compile: $(FILE)
	gcc -o $(PRGM) $(FILE) `sdl2-config --cflags --libs`