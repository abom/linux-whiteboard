#include "matrix.h"

matrix_t *newMatrix(matrix_t *m, int h, int v)
{
	m->d = malloc(h*v*sizeof(float));
	m->h = h;
	m->v = v;
	return m;
}

vector_t *newVector(vector_t *v, int h)
{
	v->d = malloc(h*sizeof(float));
	v->h = h;
	return v;
}

void freeMatrix(matrix_t *m)
{
	free(m->d);
}

void freeVector(vector_t *v)
{
	free(v->d);
}


float getVectorElement(vector_t *v, int i)
{
	return (*(v->d + i));
}

float setVectorElement(vector_t *v, float e, int i)
{
	*(v->d + i) = e;
}

float printVector(vector_t *v)
{
	int i;
	for (i=0; i<v->h; i++)
		printf("%3.2f ", getVectorElement(v,i));
	printf("\n");
}

float getMatrixElement(matrix_t *m, int i, int j)
{
	return (*(m->d + i + j*m->v));
}

void setMatrixElement(matrix_t *m, float e, int i, int j)
{
	*(m->d + i + j*m->v) = e;
}

void printMatrix(matrix_t *m)
{
	int i,j;
	for (j=0; j<m->v; j++)
	{
		for (i=0; i<m->h; i++)
			printf("%03.2f ",getMatrixElement(m,i,j));
		printf("\n");
	}
}


void calc_cofactor(matrix_t *c, matrix_t *m, int x, int y)
{
	int i,j;
	int xx = 0;
	int yy = 0;

	newMatrix(c, m->h-1, m->v-1);

	for (i=0; i<m->h; i++)
	{
		if (i==x) continue;
		yy = 0;
		for (j=0; j<m->v; j++)
		{
			if (j==y) continue;
			setMatrixElement(c, getMatrixElement(m,i,j), xx, yy);
			yy++;
		}
		xx++;
	}
}



float determinant(matrix_t *m)
{
	int i;
	float r;
	int p;
	matrix_t cofactor;

	if ((m->h == 2) && (m->v == 2))
		return (getMatrixElement(m,0,0)*getMatrixElement(m,1,1) -
			getMatrixElement(m,0,1)*getMatrixElement(m,1,0));
	
	/* Determinant calculation: laplace formula */

	r = 0.0f;
	for (i=0; i<m->h; i++)
	{
		calc_cofactor(&cofactor,m,i,0);
		if (i % 2) p=-1; else p=1;
		r = r + getMatrixElement(m,i,0) * p * determinant(&cofactor);
		freeMatrix(&cofactor);
	}

	return r;

}


void matrixCofactor(matrix_t *c, matrix_t *m)
{
	int i,j;
	float f;
	int p;
	matrix_t cc;
	newMatrix(c, m->h, m->v);

	for (i=0; i<m->h; i++)
		for (j=0; j<m->v; j++)
		{
			p = ((i+j) % 2) ? -1: 1;
			calc_cofactor(&cc,m,i,j);
			f = p*determinant(&cc);
			setMatrixElement(c,f,i,j);
		}

}

void matrixCopy(matrix_t *source, matrix_t *dest)
{
	int i,j;
	newMatrix(dest,source->h,source->v);
	
	for(i=0; i<source->h; i++)
		for(j=0; j<source->v; j++)
			setMatrixElement(dest, getMatrixElement(source,i,j), i, j);

	dest->h = source->h;
	dest->v = source->v;
}


void matrixTranspose(matrix_t *m)
{
	int i,j;
	matrix_t c;
	matrixCopy(m,&c);

	for (i=0; i<c.h; i++)
		for(j=0; j<c.v; j++)
			setMatrixElement(m, getMatrixElement(&c,i,j), j, i);

	freeMatrix(&c);
}


void matrixInverse(matrix_t *m)
{
	float det;
	matrix_t inv;
	int i,j;
	float f;

	det = determinant(m);
	matrixCofactor(&inv,m);
	matrixTranspose(&inv);

	for(i=0; i<inv.h; i++)
		for(j=0; j<inv.v; j++)
		{
			f = getMatrixElement(&inv,i,j);
			f = f / det;
			setMatrixElement(m,f,i,j);
		}
	
	freeMatrix(&inv);

}


void matrixDOTvector(matrix_t *m, vector_t *v, matrix_t *res)
{
	int i,j;
	float f;

	newMatrix(res,1,m->v);

	for (j=0; j<m->v; j++)
	{
		f = 0.0f;
		for (i=0; i<m->h; i++)
			f = f + getMatrixElement(m,i,j)*getVectorElement(v,i);
		
		setMatrixElement(res,f,0,j);
	}
}




/*


int main()
{
	int i,j;
	matrix_t m;
	matrix_t r;
	vector_t v;

	newMatrix(&m,3,3);

	setMatrixElement(&m,2.0f,0,0);
	setMatrixElement(&m,3.0f,1,0);
	setMatrixElement(&m,-1.0f,2,0);
	
	setMatrixElement(&m,8.0f,0,1);
	setMatrixElement(&m,-7.0f,1,1);
	setMatrixElement(&m,0.0f,2,1);

	setMatrixElement(&m,1.0f,0,2);
	setMatrixElement(&m,4.0f,1,2);
	setMatrixElement(&m,-6.0f,2,2);

	newVector(&v,3);
	setVectorElement(&v,5.0f,0);
	setVectorElement(&v,3.0f,1);
	setVectorElement(&v,8.0f,2);

	matrixInverse(&m);
	matrixDOTvector(&m,&v,&r);
	
	printf("\n\n\n");
	printMatrix(&r);
}


*/



