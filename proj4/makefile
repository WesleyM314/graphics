CC       = gcc 
CFLAGS   = -O3 -Wall -Wno-unused-result -Wno-maybe-uninitialized
LIBS      = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL
OBJDIR   = ../lib
OBJS     = $(OBJDIR)/initShader.o $(OBJDIR)/lib.o

proj4: proj4.c $(OBJS)
	$(CC) -o proj4 proj4.c $(OBJS) $(CFLAGS) $(LIBS)

$(OBJDIR)/%.o: %.c %.h
	$(CC) -c @< -o $@ $(CFLAGS)

run: proj4
	./proj4

