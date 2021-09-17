CC       = gcc 
CFLAGS   = -O3 -Wall 
LIBS      = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL
OBJDIR   = ../lib
OBJS     = $(OBJDIR)/initShader.o $(OBJDIR)/lib.o

lab3: lab3.c $(OBJS)
	$(CC) -o lab3 lab3.c $(OBJS) $(CFLAGS) $(LIBS)

$(OBJDIR)/%.o: %.c %.h
	$(CC) -c @< -o $@ $(CFLAGS)

