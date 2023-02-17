#include <stdio.h>
#include <math.h>
#define PI 3.14159265
#define lens 10000000

double arrsin_f[lens];

void sins_float(double* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * PI * 2 / lens);;
}

double sum_sins_float(double* arrsin) {
#pragma acc kernels
    double sum = 0;
    for (int i = 0; i < lens; i++)
        sum += arrsin[i];
    return sum;
}

int main()
{
    sins_float(arrsin_f);
    printf("%.25lf", sum_sins_float(arrsin_f));
}
