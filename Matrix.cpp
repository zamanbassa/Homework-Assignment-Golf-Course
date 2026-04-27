#include "Matrix.h"

template<int a, int b>
Matrix<a, b>::Matrix()
{
    this->arr = new float*[a];

    for(int x = 0; x < a; x++)
    {
        this->arr[x] = new float[b];

        for(int y = 0; y < b; y++)
        {
            this->arr[x][y] = 0;
        }
    }

    
}

template<int a, int b> 
Matrix<a, b>::Matrix(float ** CopyingMatrixes)
{

    this->arr = new float*[a];
     for(int x = 0; x < a; x++)
    {
        this->arr[x] = new float[b];
        for(int y = 0; y < b; y++)
        {
            this->arr[x][y] = CopyingMatrixes[x][y];
        }
    }

}

template<int a, int b>
Matrix<a, b>::Matrix(const Matrix<a,b> & NewMatrix)
{
    this->arr = new float*[a];
    for(int x = 0; x < a; x++)
    {
        this->arr[x] = new float[b];
        for(int s = 0; s < b; s++)
        {
            this->arr[x][s] = NewMatrix[x][s];
        }
    }
}


template<int a, int b>
Matrix<a, b>::~Matrix()
{
	for(int x = 0; x < a;x++)
    {
        delete[] arr[x];
    }

    delete[] arr;
}



template<int a, int b>
Matrix<a,b>& Matrix<a, b>::operator=(const Matrix<a,b>& MatrixtoCopy)
{


     if (this == &MatrixtoCopy)  
     {   
        return *this;
     }

     float** NewV = new float*[a];

     for(int p = 0; p < a; p++)
     {
        NewV[p] = new float[b];
        for(int x = 0; x < b;x++)
        {
            NewV[p][x] = MatrixtoCopy.arr[p][x];
        }
     }
   

    for(int x = 0; x < a;x++)
    {
        delete[] arr[x];
    }

    delete[] arr;           

    arr = NewV;

    return *this; 

}
  


 

template<int n, int m>
template<int a>
Matrix<n, a> Matrix<n, m>::operator*(const Matrix<m, a> TimesOperator) const
{
    Matrix<n, a> Times;

    for(int p = 0; p < n; p++)       
    {
        for(int z = 0; z < a; z++)    
        {
            Times[p][z] = 0;

            for(int s = 0; s < m; s++) 
            {
                Times[p][z] += this->arr[p][s] * TimesOperator[s][z];
            }
        }
    }

    return Times;
}


 template<int a, int b>
    Matrix<a,b> Matrix<a, b>::operator*(const float multiplywith) const
    {
         Matrix<a, b> MultiplyRes;

    for(int x = 0; x < a; x++)       
    {
        for(int c = 0; c < b; c++)    
        {
           
                MultiplyRes[x][c] = this->arr[x][c] * multiplywith;
            
        }
    }

    return MultiplyRes;
    }



    template<int a, int b>
Matrix<a, b> Matrix<a, b>::operator+(const Matrix<a,b> NewMatrixesPlus) const
{
    Matrix<a,b> PlusTwoMatrixes;

    for(int x = 0; x < a;x++)
    {
        for(int y = 0; y < b; y++)
        {
            PlusTwoMatrixes[x][y] = (this->arr[x][y] + NewMatrixesPlus.arr[x][y]);
        }
    }

    return PlusTwoMatrixes;
}



template<int a, int b>
Matrix<b, a> Matrix<a, b>::operator~() const
{
    Matrix<b, a> tranpose;

    for (int x = 0; x < a; x++)
    {
        for (int c = 0; c < b; c++)
        {
            tranpose[c][x] = this->arr[x][c];
        }
    }

    return tranpose;
}


    template<int a, int b>
int Matrix<a, b>::getN() const
{
    return a;
}


    template<int a, int b>
int Matrix<a, b>::getM() const
{
    return b;
}

 
    template<int a, int b>
float Matrix<a, b>::determinant() const
{
    float determinantOfMatrix;

    if(a == 1 && b == 1)
    {
        determinantOfMatrix = this->arr[0][0];
    }
    else if(a == 2 && b == 2)
    {
        float part1 = (this->arr[0][0] * this->arr[1][1]);
        float part2 = (this->arr[1][0] * this->arr[0][1]);
        determinantOfMatrix = (part1) - (part2);
    }
    return determinantOfMatrix;
}


