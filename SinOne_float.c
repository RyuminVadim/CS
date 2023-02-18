#include <stdio.h>
#include <math.h>

#define lens 10000000
#define Pi (3.14159265* 2 / lens)

float sumsin = 0;
float arrsin_f[lens];

void sins_float(float* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

float sum_sins(float* arrsin) {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        sumsin += arrsin[i];
    return sumsin;
}

int main()
{
#pragma acc data create(arrsin_f[:lens]) copy (sumsin) copyin(Pi)
    sins_float(arrsin_f);
    printf("%.25f", sum_sins(arrsin_f));
}
