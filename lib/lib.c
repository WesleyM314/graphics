#include <stdio.h>
#include <math.h>
#include "lib.h"

// VECTORS

/**
 * Prints a vector
 */
void printVec(vec4 *v)
{
	printf("[%.4f, %.4f, %.4f, %.4f]\n", v->x, v->y, v->z, v->w);
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