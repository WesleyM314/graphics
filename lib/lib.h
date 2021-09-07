
#ifdef __APPLE__  // include Mac OS X verions of headers

#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>

#else // non-Mac OS X operating systems

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#endif  // __APPLE__


typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
} vec4;

// Function Signatures

void printVec(vec4 *v);
vec4 multScalVec(vec4 *v, float s);
vec4 addVec(vec4 *v, vec4 *u);
vec4 subVec(vec4 *v, vec4 *u);
float magnitude(vec4 *v);
vec4 normalize(vec4 *v);
float dotVec(vec4 *v, vec4 *u);
