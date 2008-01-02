#include <stdio.h>
#include <stdlib.h>


typedef struct {
	float *d;
	int h,v;
} matrix_t;

typedef struct {
	float *d;
	int h;
} vector_t;

extern matrix_t *   matrixNew(int h, int v);
extern void         matrixFree(matrix_t *m);
extern float        matrixGetElement(matrix_t *m, int i, int j);
extern void         matrixSetElement(matrix_t *m, float e, int i, int j);
extern void         matrixInverse(matrix_t *m);
extern matrix_t *   matrixMultiply(matrix_t *m, matrix_t *n);


 
