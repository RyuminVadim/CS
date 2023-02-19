#include <stdio.h>
#include <math.h>

#define lens 10000000

double Pi = 3.14159265 * 2 / lens;
double arrsin[lens];
double sum = 0;

void create_sin() {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

void sum_sin() {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        sum += arrsin[i];
}

int main()
{
#pragma acc data create(arrsin[:lens]) copy (sum) copyin(Pi)
    create_sin();
    sum_sin();
    printf("sum = %.25lf\n", sum);
    return 0;
}
