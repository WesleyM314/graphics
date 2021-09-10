MAKEFLAGS += --silent
LIBS = -lm -lGL

all: main.c lib.o
	gcc -std=c99 -I../lib -o main main.c lib.o $(LIBS)

lib.o: 
	gcc -std=c99 -c ../lib/lib.c $(LIBS)

.PHONY: clean
clean:
	-rm -f $(wildcard *.o) main