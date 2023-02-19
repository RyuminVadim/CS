#include <stdio.h>
#include <math.h>
#include <time.h> 

#define lens 10000000

clock_t begin;
clock_t end;
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
#pragma acc data create(arrsin[:lens]) copy (sum,begin,end) copyin(Pi)

    begin = clock();

    create_sun();
    sum_sin();
    printf("%.25f \n", sum);

    end = clock();
    printf("time: %.15lf\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}
