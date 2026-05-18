#include "Vector.h"
#include "Matrix.h"

#include <cmath>

/**
 * @brief Vector class has an array of values
 */

/**
 * Constructor
 * initialises array
 * populates with 0
 */
template <int n>
Vector<n>::Vector() : arr(nullptr)
{
    arr = new float[n];

    for (int i = 0; i < n; i++)
        arr[i] = 0;
}

/**
 * Constructor
 * initialises array
 * populates with initialiser list
 * @param list - the initialiser list to populate the array with
 */
template <int n>
Vector<n>::Vector(std::initializer_list<float> list) : arr(nullptr)
{
    arr = new float[n];

    int size = static_cast<int>(list.size());
    std::initializer_list<float>::iterator ptr = list.begin();

    for (int i = 0; i < n; i++)
    {
        if (i > size)
            arr[i] = 0;
        else
        {
            arr[i] = *ptr;
            ++ptr;
        }
    }
}

/**
 * Constructor
 * initialises array with passed in parameter.
 * shallow copy
 *
 * @param array - initialises vector elements to this array
 */
template <int n>
Vector<n>::Vector(float *array) : arr(nullptr)
{
    arr = array;
}

/**
 * Destructor
 * deletes allocaed memory
 */
template <int n>
Vector<n>::~Vector()
{
    if (arr)
        delete[] arr;

    arr = nullptr;
}

/**
 * Copy Constructor
 * @param copy - vector to make a copy of
 */
template <int n>
Vector<n>::Vector(const Vector<n> &copy)
{
    arr = new float[n];

    for (int i = 0; i < n; i++)
        arr[i] = copy[i];
}

/**
 * Constructor
 * inisialises vector to passed in matrix
 * deep copy
 *
 * @param matrix - the matrix to initialise the vector with
 */
template <int n>
Vector<n>::Vector(const Matrix<n, 1> &matrix)
{
    arr = new float[n];

    for (int i = 0; i < n; i++)
    {
        arr[i] = matrix[i][0];
    }
}

/**
 * converts vector to a nx1 matrix
 *
 * @return matrix - the resultant matrix of the converted array
 */
template <int n>
Vector<n>::operator Matrix<n, 1>() const
{
    float **array = new float *[n];

    for (int i = 0; i < n; i++)
    {
        array[i] = new float[1];
        array[i][0] = arr[i];
    }

    Matrix<n, 1> matrix(array);

    return matrix;
}

/**
 * assignment operator
 * @param vec - the vector to assing this to
 *
 * @return *this
 */
template <int n>
Vector<n> &Vector<n>::operator=(const Vector<n> &vec)
{
    if (this == &vec)
        return *this;

    if (arr)
    {
        delete[] arr;
        arr = nullptr;
    }

    if (arr == nullptr)
        arr = new float[n];

    for (int i = 0; i < n; i++)
    {
        arr[i] = vec[i];
    }

    return *this;
}

/**
 * getter for size of the vector
 * @return n - size of vector
 */
template <int n>
int Vector<n>::getN() const
{
    return n;
}

/**
 * adds vectors
 * @param add - the vector to add to this
 * @return result of addition
 */

template <int n>
Vector<n> Vector<n>::operator+(const Vector<n> add) const
{
    Vector<n> result;
    if (!arr)
    {
        for (int i = 0; i < n; i++)
            result[i] = 0;
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            result[i] = arr[i] + add[i];
        }
    }

    return result;
}

/**
 * subtracts vectors
 * @param subt - the vector to subtract from this
 * @return result of subtraction
 */

template <int n>
Vector<n> Vector<n>::operator-(const Vector<n> subt) const
{
    Vector<n> result;

    if (!arr)
    {
        for (int i = 0; i < n; i++)
            result[i] = 0;
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            result[i] = arr[i] - subt[i];
        }
    }

    return result;
}

/**
 * scalar multiplication
 * @param s - scalar multiplied to this vector
 * @return new vector after multiplication is applied to this
 */
template <int n>
Vector<n> Vector<n>::operator*(const float s) const
{
    Vector<n> result;

    if (!arr)
    {
        for (int i = 0; i < n; i++)
            result[i] = 0;
    }
    else
    {
        for (int i = 0; i < n; i++)
            result[i] = arr[i] * s;
    }

    return result;
}

/**
 * dot product
 * @param dot - vector to dot with this
 * @return dot product
 */
template <int n>
float Vector<n>::operator*(const Vector<n> dot) const
{
    float dot_product = 0;

    if (!arr)
        return dot_product;

    for (int i = 0; i < n; i++)
    {
        dot_product += arr[i] * dot[i];
    }

    return dot_product;
}

/**
 * magnitude of the vector with the euclidean norm
 * @return magnitude of the vector
 */
template <int n>
float Vector<n>::magnitude() const
{
    float magnitude = 0;

    for (int i = 0; i < n; i++)
    {
        magnitude += arr[i] * arr[i];
    }

    return sqrtf(magnitude);
}

/**
 * cross product
 * this x vec
 * @param vec the vector thats crossed with this
 * @return result of cross product
 */
template <int n>
Vector<3> Vector<n>::crossProduct(const Vector<3> vec) const
{
    Vector<3> crossP;

    crossP[0] = arr[1] * vec[2] - arr[2] * vec[1];
    crossP[1] = arr[2] * vec[0] - arr[0] * vec[2];
    crossP[2] = arr[0] * vec[1] - arr[1] * vec[0];

    return crossP;
}

/**
 * return unit vector of this
 * @return unit vector
 */
template <int n>
Vector<n> Vector<n>::unitVector() const{
    float mag = magnitude();
    if(mag == 0) throw "Invalid unit vector";

    Vector<n> unit;

    for(int i = 0; i < n; i++){
        unit[i] = arr[i]/mag;
    }

    return unit;
}