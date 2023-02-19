#include <stdio.h>
#include <math.h>

#define lens 10000000

float Pi = 3.14159265 * 2 / lens;
float sumsin = 0;
float arrsin_f[lens];

void create_sun(float* arrsin) {
    #pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

float sum_sin(float* arrsin) {
    #pragma acc kernels
    for (int i = 0; i < lens; i++)
        sumsin += arrsin[i];
    return sumsin;
}

int main()
{
#pragma acc data create(arrsin_f[:lens]) copy (sumsin) copyin(Pi)
    create_sun(arrsin_f);
    printf("%.25f", sum_sin(arrsin_f));
}
