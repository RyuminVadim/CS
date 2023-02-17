#include <iostream>
#include <math.h>
#define PI 3.14159265
#define lens 10000000

void sins_float(float* arrsin) {
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * PI / 180);;
}

void sins_double(double* arrsin) {
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * PI / 180);;
}
void printsin_float(float* arrsin) {
    //#pragma acc kernels
    for (int i = 0; i < lens; i++)
        std::cout << arrsin[i] << std::endl;
}

int main()
{
    float* arrsin_f = new float[lens];
    double* arrsin_d = new double[lens];
    //std::cout << lens<< std::endl;
    //std::cout << (arrsin_f);
    sins_float(arrsin_f);
    //sins_double(arrsin_d);


    //for (int i = 0; i < lens; i++)
        //std::cout << arr[i] << std::endl;
}
