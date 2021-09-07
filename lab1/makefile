LIBS = -lXi -lXmu -lglut -lGLEW -lGLU -lm -lGL

triangle: triangle.c initShader.o
	gcc -o triangle triangle.c initShader.o $(LIBS)

triangle_ctm: triangle_ctm.c initShader.o
	gcc -o triangle_ctm triangle_ctm.c initShader.o $(LIBS)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c
