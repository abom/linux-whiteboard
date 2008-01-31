/* Copyright (C) 2008 Pere Negre
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef __MATRIX_H__
#define __MATRIX_H__


#include <vector>

#include "common.h" // ASSERT


// 2D horizontal matrix
template<typename T>
struct Matrix {
public:
    Matrix(unsigned int rows, unsigned int cols);

    T* operator[](unsigned int rows);
    T const* operator[](unsigned int rows) const;
    T* elems();
    T const* elems() const;

    Matrix<T>& invert(); // Inverts inself
    Matrix<T>& transpose(); // Transposes itself

    unsigned int rows() const;
    unsigned int cols() const;
private:
    unsigned int const m_rows;
    unsigned int const m_cols;
    std::vector<T> m_e;
};


/* Arithmetic operations */
// NOTE: Performance alerts
template<typename T>
Matrix<T> operator*(Matrix<T> const m1, Matrix<T> const m2);


/* For any other functions */
typedef double matrix_elem_t;
typedef Matrix<matrix_elem_t> matrix_t;



/* IMPLEMENTATION IMPLEMENTATION IMPLEMENTATION IMPLEMENTATION IMPLEMENTATION */
template<typename T>
Matrix<T>::Matrix(unsigned int rows, unsigned int cols) :
    m_rows(rows),
    m_cols(cols),
    m_e( rows*cols )
{
    ASSERT( (rows != 0) && (cols != 0), "Invalid rows/cols number." );
}


template<typename T>
T* Matrix<T>::operator[](unsigned int rows) {
    return &m_e.at(m_cols*rows);
}
template<typename T>
T const* Matrix<T>::operator[](unsigned int rows) const {
    return &m_e.at(m_cols*rows);
}
template<typename T>
T* Matrix<T>::elems() {
    return &m_e[0];
}
template<typename T>
T const* Matrix<T>::elems() const {
    return &m_e[0];
}


template<typename T>
Matrix<T>& Matrix<T>::invert() {
    T const det = matrixDeterminant(*this);
    Matrix const temp = matrixCofactor(*this).transpose();
    T const* temp_elems = temp.elems();

    unsigned int const n_elems = m_rows*m_cols;
    for(unsigned int i = 0; i != n_elems; ++i)
	m_e[i] = temp_elems[i] / det;

    return *this;
}
template<typename T>
Matrix<T>& Matrix<T>::transpose() {
    Matrix<T> const c = *this;

    for (unsigned int i = 0; i != m_rows; ++i)
	for(unsigned int j = 0; j != m_cols; ++j)
	    (*this)[j][i] = c[i][j];

    return *this;
}


template<typename T>
unsigned int Matrix<T>::rows() const {
    return m_rows;
}
template<typename T>
unsigned int Matrix<T>::cols() const {
    return m_cols;
}


template<typename T>
Matrix<T> operator*(Matrix<T> const m1, Matrix<T> const m2) {
    return matrixMul(m1, m2);
}


/* Helpers */
template<typename T>
void matrixPrint(Matrix<T> const& m)
{
    unsigned int const n_elems = m.rows()*m.cols();
    T const*const elems = m.elems();
    for (unsigned int i = 0; i != n_elems; ++i)
    {
	printf("%03.2f ", elems[i]);
	if (!(i / m.cols()))
	    printf ("\n");
    }
}


template<typename T>
Matrix<T> matrixCof1(Matrix<T> const& m, unsigned int x, unsigned int y)
{
    Matrix<T> c = Matrix<T>( m.rows()-1, m.cols()-1 );

    unsigned int xx = 0;
    for (unsigned int i = 0; i != m.rows(); ++i)
	if (i != x) {
	    unsigned int yy = 0;
	    for (unsigned int j = 0; j != m.cols(); ++j)
		if (j != y) {
		    c[xx][yy] = m[i][j];
		    ++yy;
		}
	    ++xx;
	}

    return c;
}
template<typename T>
T matrixDeterminant(Matrix<T> const& m)
{
    T ret = 0.0;

    if ((m.rows() == 2) && (m.cols() == 2)) {
	ret = m[0][0]*m[1][1] - m[0][1]*m[1][0];
    }
    else { // Determinant calculation: Laplace formula
	for (unsigned int i = 0; i != m.rows(); ++i)
	{
	    Matrix<T> const cofactor = matrixCof1(m, i, 0);
	    T const p = (i % 2) ? -1.0 : 1.0; // Instead of int so as it does not get promoted
	    ret += m[i][0]*p*matrixDeterminant(cofactor);
	}
    }

    return ret;
}
template<typename T>
Matrix<T> matrixCofactor(Matrix<T> const& m)
{
    Matrix<T> c = Matrix<T>( m.rows(), m.cols() );

    for (unsigned int i = 0; i != m.rows(); ++i)
	for (unsigned int j = 0; j != m.cols(); ++j)
	{
	    T const p = ((i+j) % 2) ? -1.0 : 1.0;

	    Matrix<T> const cc = matrixCof1(m, i, j);
	    T const f = p*matrixDeterminant(cc);

	    c[i][j] = f;
	}

    return c;
}


template<typename T>
Matrix<T> matrixMul(Matrix<T> const& m, Matrix<T> const& n)
{
    Matrix<T> r = Matrix<T>( n.rows(), m.cols() );

    for (unsigned int j = 0; j != r.cols(); ++j)
	for (unsigned int k = 0; k != n.rows(); ++k) {
	    T total = 0.0;
	    for (unsigned int i = 0; i != m.rows(); ++i)
		total += m[i][j] * n[k][i];
	    r[k][j] = total;
	}

    return r;
}


#endif /* __MATRIX_H__ */
