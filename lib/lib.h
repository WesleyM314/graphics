
#ifdef __APPLE__  // include Mac OS X verions of headers

#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>

#else // non-Mac OS X operating systems

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#endif  // __APPLE__

// STRUCTS

typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
} vec4;

typedef GLfloat vec4Arr[4];

typedef struct
{
	vec4 x;
	vec4 y;
	vec4 z;
	vec4 w;
} mat4;

typedef GLfloat mat4Arr[16];

// Function Signatures

void printVec(vec4 *v);
void vecToArr(vec4 *v, vec4Arr arr);
void arrToVec(vec4Arr arr, vec4 *v);
vec4 multScalVec(vec4 *v, float s);
vec4 addVec(vec4 *v, vec4 *u);
vec4 subVec(vec4 *v, vec4 *u);
float magnitude(vec4 *v);
vec4 normalize(vec4 *v);
float dotVec(vec4 *v, vec4 *u);

void printMat(mat4 *m);
mat4 multScalMat(mat4 *m, float s);
mat4 addMat(mat4 *m, mat4 *n);
mat4 subMat(mat4 *m, mat4 *n);
