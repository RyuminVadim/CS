#include <stdio.h>
#include <math.h>
#include <time.h> 

#define lens 10000000

clock_t begin;
clock_t end;
float Pi = 3.14159265 * 2 / lens;
float sum = 0;
float arrsin_f[lens];

void create_sun(float* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

float sum_sin(float* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        sum += arrsin[i];
    return sum;
}

int main()
{
#pragma acc data create(arrsin_f[:lens]) copy (sum,begin,end) copyin(Pi)

    begin = clock();

    create_sun(arrsin_f);
    printf("sum = %.25f\n", sum_sin(arrsin_f));

    end = clock();
    printf("time: %.15lf\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}
