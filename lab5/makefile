CC       = gcc 
CFLAGS   = -O3 -Wall 
LIBS      = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL
OBJDIR   = ../lib
OBJS     = $(OBJDIR)/initShader.o $(OBJDIR)/lib.o

lab05: lab05.c $(OBJS)
	$(CC) -o lab05 lab05.c $(OBJS) $(CFLAGS) $(LIBS)

$(OBJDIR)/%.o: %.c %.h
	$(CC) -c @< -o $@ $(CFLAGS)

