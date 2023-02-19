#include <stdio.h>
#include <math.h>
#include <time.h> 

#define lens 10000000

clock_t begin;
clock_t end;
double Pi = 3.14159265 * 2 / lens;
double arrsin[lens];
double sum = 0;

void create_sin() {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        arrsin[i] = sin(i * Pi);;
}

double sum_sin() {
#pragma acc kernels
    for (int i = 0; i < lens; i++)
        sum += arrsin[i];
}

int main()
{
#pragma acc data create(arrsin[:lens]) copy (sum,begin,end) copyin(Pi)

    begin = clock();

    create_sin();
    sum_sin();
    printf("sum = %.25lf\n", sum);

    end = clock();
    printf("time: %.15lf\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}
