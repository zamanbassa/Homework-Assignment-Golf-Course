
#include "Vector.h"

template<int a>
Vector<a>::Vector()
{
    this->arr = new float[a];

    for(int x = 0; x < a;x++)
    {
        this->arr[x] = 0;
    }
}

template<int a>
Vector<a>::Vector(std::initializer_list<float> list)
{
    this->arr = new float[a];
    int increment = 0;
   

    for (float value : list) 
    {

        if(increment >= a)
        {
            break;
        }

        this->arr[increment] = value;
        increment = increment + 1; 
    }

   
}
    
template<int a>
Vector<a>::Vector(float* vectories)
{
    this->arr = vectories;
}

template<int a>
Vector<a>::~Vector()
{
    delete[] arr;
}
    
template<int a>
Vector<a>::Vector(const Vector<a> &copyingArr)
{
    this->arr = new float[a];

    for(int x = 0; x < a;x++)
    {
        this->arr[x] = copyingArr.arr[x];
    }

}
    
template<int a>
Vector<a>::Vector(const Matrix<a,1>& changingToVector)
{
    this->arr = new float[a];
    for(int x = 0; x < a ;x++)
    {
        this->arr[x] = changingToVector[x][0];
    }

}
    
template<int a>
Vector<a>& Vector<a>::operator=(const Vector<a>& VectorsCopy)
{
     if (this == &VectorsCopy)  
     {   
        return *this;
     }

     float* NewV = new float[a];

    for(int x = 0; x < a;x++)
        {
            NewV[x] = VectorsCopy.arr[x]; 
        }

    delete[] arr;           

    arr = NewV;

    return *this; 

}
    
template<int a>
Vector<a> Vector<a>::operator+(const Vector<a> VectorToAdd) const
{

    Vector<a> NewAdditionOfTwoVector;

    for(int x = 0; x < a;x++)
    {
        NewAdditionOfTwoVector.arr[x] = (VectorToAdd.arr[x] + this->arr[x]);
    }

    return NewAdditionOfTwoVector;

}
    
template<int a>
Vector<a> Vector<a>::operator-(const Vector<a> VectorToSubstract) const
{

    Vector<a> NewSubstractionOfTwoVector;

    for(int z = 0; z < a;z++)
    {
        NewSubstractionOfTwoVector.arr[z] = (this->arr[z] - VectorToSubstract.arr[z]);
    }

    return NewSubstractionOfTwoVector;

}
    
template<int a>
Vector<a> Vector<a>::operator*(const float VectorToMulti) const
{

    Vector<a> NewMultpiOfTwoVector;

    for(int p = 0; p < a;p++)
    {
        NewMultpiOfTwoVector[p] = this->arr[p] * VectorToMulti ;
    }

    return NewMultpiOfTwoVector;

}
    

    template<int a>
float Vector<a>::magnitude() const
{

    float sqauredAllFIleds = 0;
    float sqrtValue = 0;

    for(int x = 0; x < a;x++)
    {
        sqauredAllFIleds = sqauredAllFIleds + (this->arr[x] * this->arr[x]);
    }


    sqrtValue = std::sqrt(sqauredAllFIleds);
    return sqrtValue;
 
}


template<int a>
Vector<a>::operator Matrix<a,1>() const
{

    Matrix<a,1> NewMatrixes;   

    for(int x = 0; x < a; x++)
    {
        NewMatrixes[x][0] = this->arr[x];
    }

    return NewMatrixes;

}




template<int a>
Vector<3> Vector<a>::crossProduct(const Vector<3> CrossProducts) const
{

    Vector<a> CrossProductRes;

    CrossProductRes.arr[0] = (this->arr[1] * CrossProducts.arr[2]) - (this->arr[2] * CrossProducts.arr[1]);
    CrossProductRes.arr[1] = (this->arr[2] * CrossProducts.arr[0]) - (this->arr[0] * CrossProducts.arr[2]);
    CrossProductRes.arr[2] = (this->arr[0] * CrossProducts.arr[1]) - (this->arr[1] * CrossProducts.arr[0]);
    
    return CrossProductRes;
    
}


template<int a>
Vector<a> Vector<a>::unitVector() const
{

    Vector<a> unitvectorRes;

    float sqauredAllFIleds = 0;

    for(int x = 0; x < a;x++)
    {
        sqauredAllFIleds = sqauredAllFIleds + (this->arr[x] * this->arr[x]);
    }

    float FindFInalvector = std::sqrt(sqauredAllFIleds);

    for(int x = 0; x < a; x++)
    {
        int divide = (this->arr[x] / FindFInalvector);
        unitvectorRes.arr[x] = divide;
    }

    return unitvectorRes;

}


template<int a>
 float Vector<a>::operator*(const Vector<a> TimesVectors) const
 {
    float timesReturn = 0;

    for(int x = 0; x < a; x++)
    {
        for(int y = 0; y < a;y++)
        {
            if(x == y)
            {
                timesReturn += TimesVectors[y] * this->arr[x];
            }
        }
    }

    return timesReturn;
 }

template<int a>
int Vector<a>::getN() const
{
    return a;
}