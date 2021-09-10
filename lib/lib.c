#include <stdio.h>
#include <math.h>
#include "lib.h"

// VECTORS

/**
 * Prints a vector
 */
void printVec(vec4 *v)
{
	printf("[%.4f\t %.4f\t %.4f\t %.4f]\n\n", v->x, v->y, v->z, v->w);
}

/**
 * Converts a vector struct to an array for 
 * places where it's easier to use loops.
 */
void vecToArr(vec4 *v, vec4Arr arr)
{
	arr[0] = v->x;
	arr[1] = v->y;
	arr[2] = v->z;
	arr[3] = v->w;
}

void arrToVec(vec4Arr arr, vec4 *v)
{
	v->x = arr[0];
	v->y = arr[1];
	v->z = arr[2];
	v->w = arr[3];
}

/**
 * Multiplies a vector by a scalar value, returning
 * a new 4x1 vector
 */
vec4 multScalVec(vec4 *v, float s)
{
	vec4 temp = {v->x * s, v->y * s, v->z * s, v->w * s};
	return temp;
}

/**
 * Adds two vectors together
 */
vec4 addVec(vec4 *v, vec4 *u)
{
	vec4 temp = {v->x + u->x, v->y + u->y, v->z + u->z, v->w + u->w};
	return temp;
}

/**
 * Returns v - u
 */
vec4 subVec(vec4 *v, vec4 *u)
{
	vec4 temp = {v->x - u->x, v->y - u->y, v->z - u->z, v->w - u->w};
	return temp;
}

/**
 * Returns the magnitude of a vector, the square root
 * of the sum of the squares of its elements.
 */
float magnitude(vec4 *v)
{
	return sqrtf((v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w));
}

/**
 * Returns a normalized copy of a vector
 */
vec4 normalize(vec4 *v)
{
	return multScalVec(v, (1 / magnitude(v)));
}

/**
 * Returns the dot product of two vectors
 */
float dotVec(vec4 *v, vec4 *u)
{
	return (v->x * u->x) + (v->y * u->y) + (v->z * u->z) + (v->w * u->w);
}

vec4 crossVec(vec4 *v, vec4 *u)
{
	return (vec4){
		(v->y * u->z - v->z * u->y),
		(v->z * u->x - v->x * u->z),
		(v->x * u->y - v->y * u->x),
		0.0};
}

// MATRICES

void printMat(mat4 *m)
{
	printf("[%-10.4f %-10.4f %-10.4f %-10.4f]\n", m->x.x, m->y.x, m->z.x, m->w.x);
	printf("[%-10.4f %-10.4f %-10.4f %-10.4f]\n", m->x.y, m->y.y, m->z.y, m->w.y);
	printf("[%-10.4f %-10.4f %-10.4f %-10.4f]\n", m->x.z, m->y.z, m->z.z, m->w.z);
	printf("[%-10.4f %-10.4f %-10.4f %-10.4f]\n\n", m->x.w, m->y.w, m->z.w, m->w.w);
}

void matToArr(mat4 *m, mat4Arr arr)
{
	
}

mat4 multScalMat(mat4 *m, float s)
{
	mat4 temp = {
		multScalVec(&m->x, s),
		multScalVec(&m->y, s),
		multScalVec(&m->z, s),
		multScalVec(&m->w, s),
	};
	return temp;
}

mat4 addMat(mat4 *m, mat4 *n)
{
	mat4 temp = {
		addVec(&m->x, &n->x),
		addVec(&m->y, &n->y),
		addVec(&m->z, &n->z),
		addVec(&m->w, &m->w),
	};
	return temp;
}

mat4 subMat(mat4 *m, mat4 *n)
{
	mat4 temp = {
		subVec(&m->x, &n->x),
		subVec(&m->y, &n->y),
		subVec(&m->z, &n->z),
		subVec(&m->w, &n->w),
	};
	return temp;
}

mat4 multMat(mat4 *m, mat4 *n)
{
	// vec4 v1 = {
	// 	m->x.x * n->x.x + m->y.x * n->x.y + m->z.x * n->x.z + m->w.x * n->x.w,

	// }
}