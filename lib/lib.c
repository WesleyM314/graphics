#include <stdio.h>
#include <math.h>
#include "lib.h"

// VECTORS

vec2 v2(GLfloat x, GLfloat y)
{
	vec2 v = {x, y};
	return v;
}

void printVec2(vec2 *v)
{
	printf("[%10.4f %10.4f]\n\n", v->x, v->y);
}

/**
 * Vector constructor
 */
vec4 v4(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	vec4 v = {x, y, z, w};
	return v;
}

/**
 * Prints a vector
 */
void printVec(vec4 *v)
{
	printf("[%10.4f %10.4f %10.4f %10.4f]\n\n", v->x, v->y, v->z, v->w);
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

/**
 * Converts an array to a vector struct
 * to help with some looping
 */
void arrToVec(vec4Arr arr, vec4 *v)
{
	v->x = arr[0];
	v->y = arr[1];
	v->z = arr[2];
	v->w = arr[3];
}

char equalVecs(vec4 *m, vec4 *n)
{
	return (m->x == n->x && m->y == n->y && m->z == n->z && m->w == n->w) ? 1 : 0;
}

char equalMats(mat4 *m, mat4 *n)
{
	return (
			   equalVecs(&m->x, &n->x) &&
			   equalVecs(&m->y, &n->y) &&
			   equalVecs(&m->z, &n->z) &&
			   equalVecs(&m->w, &n->w))
			   ? 1
			   : 0;
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

/**
 * Returns the cross product of two vectors
 */
vec4 crossVec(vec4 *v, vec4 *u)
{
	return (vec4){
		(v->y * u->z - v->z * u->y),
		(v->z * u->x - v->x * u->z),
		(v->x * u->y - v->y * u->x),
		0.0};
}

// MATRICES

/**
 * Matrix constructor
 */
mat4 m4(vec4 x, vec4 y, vec4 z, vec4 w)
{
	mat4 m = {x, y, z, w};
	return m;
}

/**
 * Prints a matrix
 */
void printMat(mat4 *m)
{
	printf("[%10.4f %10.4f %10.4f %10.4f]\n", m->x.x, m->y.x, m->z.x, m->w.x);
	printf("[%10.4f %10.4f %10.4f %10.4f]\n", m->x.y, m->y.y, m->z.y, m->w.y);
	printf("[%10.4f %10.4f %10.4f %10.4f]\n", m->x.z, m->y.z, m->z.z, m->w.z);
	printf("[%10.4f %10.4f %10.4f %10.4f]\n\n", m->x.w, m->y.w, m->z.w, m->w.w);
}

/**
 * Converts a matrix to an array to 
 * help with looping
 */
void matToArr(mat4 *m, mat4Arr arr)
{
	arr[0] = m->x.x;
	arr[1] = m->x.y;
	arr[2] = m->x.z;
	arr[3] = m->x.w;

	arr[4] = m->y.x;
	arr[5] = m->y.y;
	arr[6] = m->y.z;
	arr[7] = m->y.w;

	arr[8] = m->z.x;
	arr[9] = m->z.y;
	arr[10] = m->z.z;
	arr[11] = m->z.w;

	arr[12] = m->w.x;
	arr[13] = m->w.y;
	arr[14] = m->w.z;
	arr[15] = m->w.w;
}

/**
 * Convers an array to a matrix
 * to help with looping
 */
mat4 arrToMat(mat4Arr arr)
{
	mat4 temp = {
		{arr[0], arr[1], arr[2], arr[3]},
		{arr[4], arr[5], arr[6], arr[7]},
		{arr[8], arr[9], arr[10], arr[11]},
		{arr[12], arr[13], arr[14], arr[15]},
	};

	return temp;
}

/**
 * Returns the identity matrix.
 */
mat4 identity(void)
{
	mat4 temp = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1},
	};
	return temp;
}

/**
 * Multiplies a matrix by a scalar value
 */
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

/**
 * Adds to matrixes together
 */
mat4 addMat(mat4 *m, mat4 *n)
{
	mat4 temp = {
		addVec(&m->x, &n->x),
		addVec(&m->y, &n->y),
		addVec(&m->z, &n->z),
		addVec(&m->w, &n->w),
	};
	return temp;
}

/**
 * Subtracts matrix n from m, 
 * giving m - n
 */
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

/**
 * Multiplies two matrixes together
 */
mat4 multMat(mat4 *m, mat4 *n)
{
	// Transpose m so we can easily
	// use dot products to get values
	mat4 t = transpose(m);
	mat4 r;

	// Use dot products of the appropriate
	// vectors to fill values of result matrix
	r.x.x = dotVec(&t.x, &n->x);
	r.x.y = dotVec(&t.y, &n->x);
	r.x.z = dotVec(&t.z, &n->x);
	r.x.w = dotVec(&t.w, &n->x);

	r.y.x = dotVec(&t.x, &n->y);
	r.y.y = dotVec(&t.y, &n->y);
	r.y.z = dotVec(&t.z, &n->y);
	r.y.w = dotVec(&t.w, &n->y);

	r.z.x = dotVec(&t.x, &n->z);
	r.z.y = dotVec(&t.y, &n->z);
	r.z.z = dotVec(&t.z, &n->z);
	r.z.w = dotVec(&t.w, &n->z);

	r.w.x = dotVec(&t.x, &n->w);
	r.w.y = dotVec(&t.y, &n->w);
	r.w.z = dotVec(&t.z, &n->w);
	r.w.w = dotVec(&t.w, &n->w);

	return r;
}

/**
 * Transposes a matrix
 */
mat4 transpose(mat4 *m)
{
	mat4 temp = {
		{m->x.x, m->y.x, m->z.x, m->w.x},
		{m->x.y, m->y.y, m->z.y, m->w.y},
		{m->x.z, m->y.z, m->z.z, m->w.z},
		{m->x.w, m->y.w, m->z.w, m->w.w},
	};
	return temp;
}

/**
 * Get the inverse of a matrix
 */
mat4 invMat(mat4 *m)
{
	// Identity matrix
	// mat4 I = {
	// 	{1, 0, 0, 0},
	// 	{0, 1, 0, 0},
	// 	{0, 0, 1, 0},
	// 	{0, 0, 0, 1},
	// };

	// Matrix of minor
	mat4 minor = minorMat(m);
	// Matrix of cofactor
	mat4 cofac = cofactor(&minor);
	// Transpose cofactor
	mat4 t = transpose(&cofac);
	// Determinant of m
	GLfloat det = m->x.x * minor.x.x - m->y.x * minor.y.x + m->z.x * minor.z.x - m->w.x * minor.w.x;
	// Inverse of m
	mat4 invM = multScalMat(&t, 1 / det);

	return invM;
}

/**
 * Get the matrix of a minor as outlined in slides
 */
mat4 minorMat(mat4 *m)
{
	mat4Arr mArr;
	matToArr(m, mArr);
	GLfloat temp[9];
	mat4Arr minor;

	// Col and Row of matrix of minor
	for (int c = 0; c < 4; c++)
	{
		for (int r = 0; r < 4; r++)
		{
			// Loop through m, ignore r and c to get
			// determinant of 3x3 matrix to put in minor
			int index = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					if (i == c || j == r)
						continue;
					temp[index] = mArr[j + i * 4];
					index++;
				}
			}
			if (index != 9)
			{
				printf("\n\nINDEX != 9 IN minorMat\n\n");
			}
			// Temp should be populated; get the determinant
			// and put it in minor
			minor[r + c * 4] = det3x3(temp);
		}
	}

	// Minor should be populated; convert to mat4 and return
	return arrToMat(minor);
}

/**
 * Get the cofactor matrix as outlined in the slides
 */
mat4 cofactor(mat4 *m)
{
	mat4 r = *m;
	r.x.y *= -1;
	r.x.w *= -1;

	r.y.x *= -1;
	r.y.z *= -1;

	r.z.y *= -1;
	r.z.w *= -1;

	r.w.x *= -1;
	r.w.z *= -1;

	return r;
}

/**
 * Get the determinant of a 3x3 matrix
 * to help in matrix inversion
 */
GLfloat det3x3(GLfloat arr[])
{
	GLfloat r = 0;
	r += arr[0] * arr[4] * arr[8];
	r += arr[3] * arr[7] * arr[2];
	r += arr[6] * arr[1] * arr[5];
	r -= arr[2] * arr[4] * arr[6];
	r -= arr[5] * arr[7] * arr[0];
	r -= arr[8] * arr[1] * arr[3];

	return r;
}

/**
 * Multiply a vector and a matrix together
 */
vec4 multMatVec(mat4 *m, vec4 *v)
{
	mat4 t = transpose(m);
	vec4 r = {
		dotVec(&t.x, v),
		dotVec(&t.y, v),
		dotVec(&t.z, v),
		dotVec(&t.w, v),
	};
	return r;
}

// AFFINE TRANSFORMATIONS

/**
 * Returns a translation matrix
 */
mat4 translate(GLfloat x, GLfloat y, GLfloat z)
{
	mat4 t = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{x, y, z, 1},
	};

	return t;
}

/**
 * Returns a scaling matrix
 */
mat4 scale(GLfloat x, GLfloat y, GLfloat z)
{
	mat4 s = {
		{x, 0, 0, 0},
		{0, y, 0, 0},
		{0, 0, z, 0},
		{0, 0, 0, 1},
	};

	return s;
}

// ROTATIONS

/**
 * Returns matrix to rotate around x axis.
 * Theta is in radians
 */
mat4 x_rotate(GLfloat theta)
{
	mat4 r = {
		{1, 0, 0, 0},
		{0, cos(theta), sin(theta), 0},
		{0, -sin(theta), cos(theta), 0},
		{0, 0, 0, 1},
	};

	return r;
}

/**
 * Returns matrix to rotate around y axis.
 * Theta is in radians
 */
mat4 y_rotate(GLfloat theta)
{
	mat4 r = {
		{cos(theta), 0, -sin(theta), 0},
		{0, 1, 0, 0},
		{sin(theta), 0, cos(theta), 0},
		{0, 0, 0, 1},
	};

	return r;
}

/**
 * Returns matrix to rotate around z axis.
 * Theta is in radians
 */
mat4 z_rotate(GLfloat theta)
{
	mat4 r = {
		{cos(theta), sin(theta), 0, 0},
		{-sin(theta), cos(theta), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1},
	};

	return r;
}

// PERSPECTIVE FUNCTIONS

/**
 * Returns transformation matrix to look at
 * a certain point
 */
mat4 look_at(vec4 eye, vec4 at, vec4 up)
{
	mat4 m;

	// VPN is eye - at; normalize to n
	vec4 n = subVec(&eye, &at);
	n = normalize(&n);

	// u is (up cross n) normalized
	vec4 u = crossVec(&up, &n);
	u = normalize(&u);

	// v is (n cross u) normalized
	vec4 v = crossVec(&n, &u);
	v = normalize(&v);

	// Make rotation matrix
	mat4 R = m4(u, v, n, v4(0, 0, 0, 1));

	R = transpose(&R);

	// Make translation matrix
	mat4 T = m4(
		v4(1, 0, 0, -eye.x),
		v4(0, 1, 0, -eye.y),
		v4(0, 0, 1, -eye.z),
		v4(0, 0, 0, 1));
	T = transpose(&T);

	T = translate(-eye.x, -eye.y, -eye.z);

	// printf("LOOK_AT T:\n");
	// printMat(&T);

	m = multMat(&R, &T);

	return m;
}

/**
 * Return a perspective projection matrix 
 */
mat4 perspective(GLfloat left, GLfloat right, GLfloat bottom,
				 GLfloat top, GLfloat near, GLfloat far)
{
	vec4 x, y, z, w;
	x = v4((-2 * near) / (right - left), 0, 0, 0);
	y = v4(0, (-2 * near) / (top - bottom), 0, 0);
	z = v4(
		(left + right) / (right - left),
		(bottom + top) / (top - bottom),
		(near + far) / (far - near), -1);
	w = v4(0, 0, (-2 * near * far) / (far - near), 0);

	return m4(x, y, z, w);
}

// FUNCTIONS FOR DYNAMIC LISTS

// Init v4List
void v4ListNew(v4List *list)
{
	list->capacity = 512;
	list->items = (vec4 *)malloc(sizeof(vec4) * list->capacity);
	list->length = 0;
}

// Resize v4List
void v4ListResize(v4List *list, int capacity)
{
	if (list)
	{
		vec4 *items = (vec4 *)realloc(list->items, sizeof(vec4) * capacity);
		if (items)
		{
			list->items = items;
			list->capacity = capacity;
			return;
		}
		printf("Error reallocating memory for v4List\n");
		exit(1);
	}
}

// Add to v4List
void v4ListPush(v4List *list, vec4 item)
{
	if (list)
	{
		// Check if list is full
		if (list->capacity == list->length)
		{
			// Double in size
			v4ListResize(list, list->capacity * 2);
		}
		// Add to end of list
		list->items[list->length] = item;
		list->length += 1;
	}
}

// Init v2List
void v2ListNew(v2List *list)
{
	list->capacity = 512;
	list->items = (vec2 *)malloc(sizeof(vec2) * list->capacity);
	list->length = 0;
}

// Resize v2List
void v2ListResize(v2List *list, int capacity)
{
	if (list)
	{
		vec2 *items = (vec2 *)realloc(list->items, sizeof(vec2) * capacity);
		if (items)
		{
			list->items = items;
			list->capacity = capacity;
			return;
		}
		printf("Error reallocating memory for v2List\n");
		exit(1);
	}
}

// Add to v2List
void v2ListPush(v2List *list, vec2 item)
{
	if (list)
	{
		// Check if list is full
		if (list->capacity == list->length)
		{
			// Double in size
			v2ListResize(list, list->capacity * 2);
		}
		// Add to end of list
		list->items[list->length] = item;
		list->length += 1;
	}
}
