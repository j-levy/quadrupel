
EXEC=js_sdl
LDFLAGS= -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib64 -lSDL2 -Wl,--no-undefined -lm -ldl -lm -ldl -lpthread -lpthread -lrt


all: compile
	./$(EXEC)

compile: $(EXEC)
	gcc -o $(EXEC) $(EXEC).c $(LDFLAGS)

clean: $(EXEC)
	rm -rf *.o $(EXEC)

	