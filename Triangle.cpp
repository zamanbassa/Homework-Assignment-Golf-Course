#include "Triangle.h"

template <int n>
Triangle<n>::Triangle(const Vector<n> &p1, const Vector<n> &p2, const Vector<n> &p3)
{
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
}

template <int n>
Triangle<n>::Triangle(const Triangle<n> & Triangles)
{
        

         this->p1 = Triangles.p1;
        this->p2 = Triangles.p2;
        this->p3 = Triangles.p3;
}

template <int n>
Triangle<n>& Triangle<n>::operator*=(const Matrix<n, n>& M)
{


    Matrix<n,1> convertP1 = (Matrix<n,1>)p1;
    Matrix<n,1> convertP2 = (Matrix<n,1>)p2;
    Matrix<n,1> convertP3 = (Matrix<n,1>)p3;

    p1 = ( M * convertP1 );
    p2 = ( M * convertP2 );
    p3 = ( M * convertP3 );

    return *this;
}


template <int n>
Triangle<n> *Triangle<n>::operator*(const Matrix<n, n> & M) const
{
     Triangle<n>* MakeACopy = new Triangle<n>(*this); 
      (*MakeACopy) *= M;                        
    return MakeACopy;
}

template <int n>
float *Triangle<n>::getPoints() const
{
        float NumberOfN = n;
            int increments = 0;
             float* returningOfTriangle = new float[3 * n];


            
            for(int a = 0; a < NumberOfN; a++)
            {
                returningOfTriangle[increments] = p1[a];
                increments++;
            }

            for(int a = 0; a < NumberOfN; a++)
            {
                returningOfTriangle[increments] = p2[a];
                increments++;
            }

            for(int a = 0; a < NumberOfN; a++)
            {
                returningOfTriangle[increments] = p3[a];
                increments++;
            }

          

    return returningOfTriangle;
}

template <int n>
int Triangle<n>::getNumPoints() const
{
        int gettingThePoints = 3 * n;
            return gettingThePoints;
}