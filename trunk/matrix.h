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


extern matrix_t *newMatrix(matrix_t *m, int h, int v);

extern vector_t *newVector(vector_t *v, int h);

extern void freeMatrix(matrix_t *m);
extern void freeVector(vector_t *v);
extern float getVectorElement(vector_t *v, int i);

extern float setVectorElement(vector_t *v, float e, int i);

extern float printVector(vector_t *v);

extern float getMatrixElement(matrix_t *m, int i, int j);

extern void setMatrixElement(matrix_t *m, float e, int i, int j);

extern void printMatrix(matrix_t *m);


extern float determinant(matrix_t *m);


extern void matrixCopy(matrix_t *source, matrix_t *dest);

extern void matrixTranspose(matrix_t *m);

extern void matrixInverse(matrix_t *m);

extern void matrixDOTvector(matrix_t *m, vector_t *v, matrix_t *res);
