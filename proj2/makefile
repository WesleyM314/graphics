CC       = gcc 
CFLAGS   = -O3 -Wall -Wno-unused-result
LIBS      = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL
OBJDIR   = ../lib
OBJS     = $(OBJDIR)/initShader.o $(OBJDIR)/lib.o

run: proj2
	./proj2

proj2: proj2.c $(OBJS)
	$(CC) -o proj2 proj2.c $(OBJS) $(CFLAGS) $(LIBS)

$(OBJDIR)/%.o: %.c %.h
	$(CC) -c @< -o $@ $(CFLAGS)

