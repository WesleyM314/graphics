MAKEFLAGS += --silent
LIBS = -lm -lGL

all: main.c lib.o
	gcc -std=c99 -I../lib -o main.o main.c $(wildcard ../lib/*.c) $(LIBS)

lib.o: 
	mkdir -p bin
	gcc -std=c99 -c ../lib/lib.c -o bin/lib.o $(LIBS)

.PHONY: clean
clean:
	-rm -f $(wildcard *.o) $(wildcard bin/*.o) main
