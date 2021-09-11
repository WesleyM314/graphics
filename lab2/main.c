#include <stdio.h>
#include "lib.h"

int main(void)
{
	GLfloat s = 3.0;

	vec4 v1 = {1, 2, 3, 4};
	vec4 v2 = {5, 6, 7, 8};
	printf("v1\n");
	printVec(&v1);
	printf("v2\n");
	printVec(&v2);

	mat4 m1 = {
		{1, -5, 9, 13},
		{2, 6, -10, 14},
		{3, 7, 11, 15},
		{4, 8, 12, -16},
	};
	mat4 m2 = {
		{4, 8, 12, 16},
		{3, 7, 11, 15},
		{2, 6, 10, 14},
		{1, 5, 9, 13},
	};
	printf("m1\n");
	printMat(&m1);
	printf("m2\n");
	printMat(&m2);
	
	printf("----------------------------------------------\n\n");

	{
		printf("s × v1\n");
		vec4 v = multScalVec(&v1, s);
		printVec(&v);
	}

	{
		printf("v1 + v2\n");
		vec4 v = addVec(&v1, &v2);
		printVec(&v);
	}

	{
		printf("v1 - v2\n");
		vec4 v = subVec(&v1, &v2);
		printVec(&v);
	}

	{
		GLfloat r = magnitude(&v1);
		printf("|v1|\n%f\n\n", r);
	}

	{
		printf("v1 normalized\n");
		vec4 v = normalize(&v1);
		printVec(&v);
	}

	{
		printf("v1 · v2\n");
		GLfloat r = dotVec(&v1, &v2);
		printf("%f\n\n", r);
	}

	{
		printf("v1 × v2\n");
		vec4 v = crossVec(&v1, &v2);
		printVec(&v);
	}

	{
		printf("s × m1\n");
		mat4 m = multScalMat(&m1, s);
		printMat(&m);
	}

	{
		printf("m1 + m2\n");
		mat4 m = addMat(&m1, &m2);
		printMat(&m);
	}

	{
		printf("m1 - m2\n");
		mat4 m = subMat(&m1, &m2);
		printMat(&m);
	}

	{
		printf("m1 × m2\n");
		mat4 m = multMat(&m1, &m2);
		printMat(&m);
	}

	{
		printf("Inverse of m1\n");
		mat4 m = invMat(&m1);
		printMat(&m);

		printf("Checking inverse of m1\n");
		mat4 I = multMat(&m1, &m);
		printMat(&I);
	}

	{
		printf("Transpose of m1\n");
		mat4 m = transpose(&m1);
		printMat(&m);
	}

	{
		printf("m1 × v1\n");
		vec4 v = multMatVec(&m1, &v1);
		printVec(&v);
	}
}
