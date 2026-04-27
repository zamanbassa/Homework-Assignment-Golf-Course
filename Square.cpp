#include "Square.h"

template <int n>
  Square<n>::Square(const Vector<n>& center, float height, float width)
  {
        // this.center = center;
        // this.h
  }

template <int n>
        Square<n>::Square(const Vector<n>& tl, const Vector<n>& tr, const Vector<n>& br, const Vector<n>& bl)
        {

            this->tl = tl;
            this->tr = tr;
            this->br = br;
            this->bl = bl;
        }
    
template <int n>
        Square<n>::Square(const Square<n>& Squares)
        {
            
            this->tl = Squares.tl;
            this->tr = Squares.tr;
            this->br = Squares.br;
            this->bl = Squares.bl;
        }

template <int n>
Square<n>& Square<n>::operator*=(const Matrix<n,n>& M)
{

    Matrix<n,1> converttl = (Matrix<n,1>)tl;
    Matrix<n,1> converttr = (Matrix<n,1>)tr;
    Matrix<n,1> convertbr = (Matrix<n,1>)br;
    Matrix<n,1> convertbl = (Matrix<n,1>)bl;

    tl = ( M * converttl );
    tr = ( M * converttr );
    br = ( M * convertbr );
    bl = ( M * convertbl );

    return *this;
}


template <int n>
Square<n>* Square<n>::operator*(const Matrix<n,n>& M) const
{
    Square<n>* TimesOperator = new Square<n>(*this); 
    (*TimesOperator) *= M;                        
    return TimesOperator;
}

template <int n>
         float* Square<n>::getPoints() const
         {

           
            float NumberOfN = n;
            int increments = 0;
             float* returning = new float[4 * n];


            
            for(int a = 0; a < NumberOfN; a++)
            {
                returning[increments] = tl[a];
                increments++;
            }

            for(int a = 0; a < NumberOfN; a++)
            {
                returning[increments] = tr[a];
                increments++;
            }

            for(int a = 0; a < NumberOfN; a++)
            {
                returning[increments] = br[a];
                increments++;
            }

            for(int a = 0; a < NumberOfN; a++)
            {
                returning[increments] = bl[a];
                increments++;
            }

    return returning;

         }

template <int n>
         int Square<n>::getNumPoints() const
         {

            int gettingThePoints = 4 * n;
            return gettingThePoints;

         }

