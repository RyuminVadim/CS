#include <stdio.h>
#include <math.h>

#define lens 10000000

double Pi(3.14159265 * 2 / lens);
double arrsin_f[lens];
double sum = 0;

void sins_float(double* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

double sum_sins(double* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        sum += arrsin[i];
    return sum;
}

int main()
{
#pragma acc data create(arrsin_f[:lens]) copy (sum) copyin(Pi)
    sins_float(arrsin_f);
    printf("%.25lf", sum_sins(arrsin_f));
}
