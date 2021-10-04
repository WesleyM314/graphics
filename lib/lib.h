
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

vec4 v4(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void printVec(vec4 *v);
void vecToArr(vec4 *v, vec4Arr arr);
void arrToVec(vec4Arr arr, vec4 *v);
char equalVecs(vec4 *m, vec4 *n);
vec4 multScalVec(vec4 *v, float s);
vec4 addVec(vec4 *v, vec4 *u);
vec4 subVec(vec4 *v, vec4 *u);
float magnitude(vec4 *v);
vec4 normalize(vec4 *v);
float dotVec(vec4 *v, vec4 *u);
vec4 crossVec(vec4 *v, vec4 *u);

mat4 m4(vec4 x, vec4 y, vec4 z, vec4 w);
void printMat(mat4 *m);
void matToArr(mat4 *m, mat4Arr arr);
mat4 arrToMat(mat4Arr arr);
mat4 identity(void);
mat4 multScalMat(mat4 *m, float s);
mat4 addMat(mat4 *m, mat4 *n);
mat4 subMat(mat4 *m, mat4 *n);
mat4 multMat(mat4 *m, mat4 *n);
mat4 transpose(mat4 *m);
mat4 invMat(mat4 *m);
mat4 minorMat(mat4 *m);
mat4 cofactor(mat4 *m);
GLfloat det3x3(GLfloat arr[]);
vec4 multMatVec(mat4 *m, vec4 *v);

mat4 translate(GLfloat x, GLfloat y, GLfloat z);
mat4 scale(GLfloat x, GLfloat y, GLfloat z);
mat4 x_rotate(GLfloat theta);
mat4 y_rotate(GLfloat theta);
mat4 z_rotate(GLfloat theta);