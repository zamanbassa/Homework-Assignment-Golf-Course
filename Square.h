#ifndef SQUARE_H
#define SQUARE_H

#include "Shape.h"
#include "Matrix.h"
#include "Vector.h"

template <int n>
class Square: public Shape<n> {
    private:
        Vector<n> tl;
        Vector<n> tr;
        Vector<n> br;
        Vector<n> bl;
    public:
        Square(const Vector<n>& center, float height, float width);
        Square(const Vector<n>& tl, const Vector<n>& tr, const Vector<n>& br, const Vector<n>& bl);
        Square(const Square<n>&);
        virtual Square<n>& operator*=(const Matrix<n,n>&);
        virtual Square<n>* operator*(const Matrix<n,n>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;

        virtual void print() const{
            std::cout << "_ P1 _ " << std::endl;
            tl.print();
            std::cout << "_ P2 _ " << std::endl;
            tr.print();
            std::cout << "_ P3 _ " << std::endl;
            br.print();
            std::cout << "_ P4 _ " << std::endl;
            bl.print();
        }
};

#include "Square.cpp"

#endif /*SQUARE_H*/
