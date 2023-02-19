#include <stdio.h>
#include <math.h>
#include <time.h> 

#define lens 10000000

float Pi = 3.14159265 * 2 / lens;
float arrsin[lens];
float sum = 0;

void create_sun() {
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
    create_sun();
    sum_sin();
    printf("sum = %.25f \n", sum);
    return 0;
}
