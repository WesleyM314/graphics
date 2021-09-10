#include <stdio.h>
#include "lib.h"

int main(void)
{
	vec4 v = {1.0, 2.0, 3.0, 4.0};
	printVec(&v);
	vec4Arr a;
	vecToArr(&v, a);
	for(int i = 0; i < 4; i++){
		printf("%.4f\t", a[i]);
	}
	printf("\n");

	vec4 v1;
	arrToVec(a, &v1);
	printVec(&v1);

	// MATRIX
	mat4 m = {{1,-5,9,13},{2,6,-10,14},{3,7,11,15},{4,8,12,-16}};
	printMat(&m);
}

