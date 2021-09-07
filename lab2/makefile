MAKEFLAGS += --silent
LIBS = -lm -lGL

all: main.c lib.o
	gcc -I../lib -o main main.c lib.o $(LIBS)

lib.o: 
	gcc -c ../lib/lib.c $(LIBS)

.PHONY: clean
clean:
	-rm -f $(wildcard *.o) main