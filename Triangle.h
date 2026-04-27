#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Shape.h"
#include "Vector.h"

template <int n>
class Triangle: public Shape<n> {
    private:
        Vector<n> p1;
        Vector<n> p2;
        Vector<n> p3;
    public:
        Triangle(const Vector<n>& p1, const Vector<n>& p2, const Vector<n>& p3);
        Triangle(const Triangle<n>&);
        virtual Triangle<n>& operator*=(const Matrix<n,n>&);
        virtual Triangle<n>* operator*(const Matrix<n,n>&) const;
        virtual float* getPoints() const;
        virtual int getNumPoints() const;

        virtual void print() const{
            std::cout << "_ P1 _ " << std::endl;
            p1.print();
            std::cout << "_ P2 _ " << std::endl;
            p2.print();
            std::cout << "_ P3 _ " << std::endl;
            p3.print();
        }
};

#include "Triangle.cpp"

#endif /*TRIANGLE_H*/
