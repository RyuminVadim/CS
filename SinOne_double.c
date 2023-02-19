#include <stdio.h>
#include <math.h>

#define lens 10000000

double Pi = 3.14159265 * 2 / lens;
double arrsin_f[lens];
double sum = 0;

void create_sin(double* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

double sum_sin(double* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        sum += arrsin[i];
    return sum;
}

int main()
{
#pragma acc data create(arrsin_f[:lens]) copy (sum) copyin(Pi)
    create_sin(arrsin_f);
    printf("%.25lf", sum_sin(arrsin_f));
}
