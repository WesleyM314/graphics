CC       = gcc 
CFLAGS   = -O3 -Wall -Wno-unused-result -Wno-maybe-uninitialized
LIBS      = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL
OBJDIR   = ../lib
OBJS     = $(OBJDIR)/initShader.o $(OBJDIR)/lib.o

proj2: proj2.c $(OBJS)
	$(CC) -o proj2 proj2.c $(OBJS) $(CFLAGS) $(LIBS)

$(OBJDIR)/%.o: %.c %.h
	$(CC) -c @< -o $@ $(CFLAGS)

file: proj2
	./proj2 file

ball: proj2
	./proj2 ball
