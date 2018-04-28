
PRGM=joytest
FILE=jstest.c

all: compile
	./$(PRGM) --normal /dev/input/js0

compile: $(FILE)
	gcc -o $(PRGM) $(FILE)