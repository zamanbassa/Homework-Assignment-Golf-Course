#ifndef MATRIX_CPP
#define MATRIX_CPP

#include "Matrix.h"

/**
 * @brief Row major 2D matrix (n x m)
 */

/**
 * Constructor
 * initialises arrar and populates with 0
 */
template <int n, int m>
Matrix<n, m>::Matrix() : arr(nullptr)
{
    arr = new float *[n];

    for (int i = 0; i < n; i++)
    {
        arr[i] = new float[m];

        for (int j = 0; j < m; j++)
        {

            if (i == j)
                arr[i][j] = 1;
            else
                arr[i][j] = 0;
        }
    }
}

/**
 * Constructor
 * initialise array and populate with passed in param
 * makes a shallow copy
 * @param init - matrix to populate array with
 */
template <int n, int m>
Matrix<n, m>::Matrix(float **init) : arr(nullptr)
{
    arr = init;
}

/**
 * Copy Constructor
 * @param cpy = matrix to copy
 */
template <int n, int m>
Matrix<n, m>::Matrix(const Matrix<n, m> &cpy)
{
    arr = new float *[n];

    for (int i = 0; i < n; i++)
    {
        arr[i] = new float[m];

        for (int j = 0; j < m; j++)
        {
            arr[i][j] = cpy[i][j];
        }
    }
}

/**
 * Destructor
 * deallocates memory of array
 */
template <int n, int m>
Matrix<n, m>::~Matrix()
{
    if (arr)
    {
        for (int i = 0; i < n; i++)
        {
            if (arr[i])
            {
                delete[] arr[i];
                arr[i] = nullptr;
            }
        }

        delete[] arr;
        arr = nullptr;
    }
}

/**
 * returns n param for template
 * @return n
 */
template <int n, int m>
int Matrix<n, m>::getN() const
{
    return n;
}

/**
 * return m param for template
 * @return m
 */
template <int n, int m>
int Matrix<n, m>::getM() const
{
    return m;
}

/**
 * Assignment operator
 * @param matrix - matrix to assign this to
 * @return this
 */
template <int n, int m>
Matrix<n, m> &Matrix<n, m>::operator=(const Matrix<n, m> &matrix)
{
    if (this == &matrix)
    {
        return *this;
    }

    if (arr)
    {
        for (int i = 0; i < n; i++)
        {
            if (arr[i])
                delete[] arr[i];
        }

        delete[] arr;
    }

    arr = new float *[n];
    for (int i = 0; i < n; i++)
    {
        arr[i] = new float[m];

        for (int j = 0; j < m; j++)
        {
            arr[i][j] = matrix[i][j];
        }
    }

    return *this;
}

/**
 * matrix multiplication
 * this * mul
 * @param mul matrix to multiply to this;
 * @return result of multiplication
 */
template <int n, int m>
template <int a>
Matrix<n, a> Matrix<n, m>::operator*(const Matrix<m, a> mul) const
{
    Matrix<n, a> result;

    int rows = n;
    int cols = a;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < m; k++)
            {
                result[i][j] += arr[i][k] * mul[k][j];
            }
        }
    }

    return result;
}

/**
 * Scale matrix by passed in parameter
 * @param s factor to scale matrix by
 * @return scaled matrix
 */
template <int n, int m>
Matrix<n, m> Matrix<n, m>::operator*(const float s) const
{
    Matrix<n, m> scale;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            scale[i][j] = arr[i][j] * s;
        }
    }

    return scale;
}

/**
 * add passed in matrix with this
 * @param matrix - added to this
 * @return - resultof addition
 */
template <int n, int m>
Matrix<n, m> Matrix<n, m>::operator+(const Matrix<n, m> add) const
{
    Matrix<n, m> result;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            result[i][j] = arr[i][j] + add[i][j];
        }
    }

    return result;
}

/**
 * Transposes this matrx
 * @return the transpose of this
 */
template <int n, int m>
Matrix<m, n> Matrix<n, m>::operator~() const
{
    Matrix<m, n> transpose;

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            transpose[i][j] = arr[j][i];
        }
    }
    return transpose;
}

/**
 * Helper function to create a submatrix from the passed in matrix
 * @param col - col of matrix to exclude for submatrix
 * @param matrix - the matrix to create a submatrix from
 *
 * the function will start at position (row,col) in the passed in matrix and create a dxd submatrix from that point
 *
 * @return the generated submatrix
 *
 */
template <int n, int m>
Matrix<n - 1, n - 1> subMatrix(int col, Matrix<n, m> matrix)
{
    ;
    Matrix<n - 1, n - 1> sub;

    if (col >= n || col < 0)
        throw "Invalid index";

    /// i,j iterate through the passed in matrix
    /// a,b iterate through the submatrix to populate
    for (int i = 1, a = 0; i < n && a < n - 1; i++)
    {

        for (int j = 0, b = 0; j < m && b < n - 1; j++)
        {
            if (j == col)
                continue;
            sub[a][b++] = matrix[i][j];
        }
        ++a;
    }

    return sub;
}

/**
 * return the determinant of this matrix
 */
template <int n, int m>
float Matrix<n, m>::determinant() const
{
    if (n != m)
        throw "Matrix is not square";

    float det = 0;

    for (int i = 0; i < n; i++)
    {
        Matrix<n - 1, n - 1> sub = subMatrix(i, *this);
        float temp = arr[0][i] * sub.determinant();

        if (i % 2 == 0)
            det -= temp;
        else
            det += temp;
    }

    return det;
}

/**
 * Base case 1
 * Specialised function for recurison
 * @return 1
 */
template <>
float Matrix<1, 1>::determinant() const
{
    return 1;
};

/**
 * Base Case 2
 * Specialised function for recursion
 * @return determinant of 2x2 matrix
 */
template <>
float Matrix<2, 2>::determinant() const
{
    return arr[0][0] * arr[1][1] - arr[0][1] * arr[1][0];
}

#endif