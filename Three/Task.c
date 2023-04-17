#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cublas_v2.h>
#include <openacc.h>
#include <cuda_runtime.h>

#define IDX2C(i ,j, ld) (((j)*(ld))+(i))

int sizearr;
double* A;
double* Anew;

//меняю массивы местами
void splits() {
	double* split = A;
	A = Anew;
	Anew = split;
}

    //заполняю массив начальными значениями 
void completionArr() {
    double step = 10 / ((double)sizearr - 1);

    A[IDX2C(0, 0, sizearr)] = 10;
	A[IDX2C(0, sizearr-1, sizearr)] = 20;
	A[IDX2C(sizearr-1, 0, sizearr)] = 20;
	A[IDX2C(sizearr-1, sizearr-1, sizearr)] = 30;

    Anew[IDX2C(0, 0, sizearr)] = 10;
	Anew[IDX2C(0, sizearr-1, sizearr)] = 20;
	Anew[IDX2C(sizearr-1, 0, sizearr)] = 20;
	Anew[IDX2C(sizearr-1, sizearr-1, sizearr)] = 30;

    for (int i = 1; i < sizearr-1; i++)
		{
            A[IDX2C(i, 0, sizearr)] = A[IDX2C(0, 0, sizearr)] + step*i;
            A[IDX2C(0, i, sizearr)] = A[IDX2C(0, 0, sizearr)] + step*i;
	        A[IDX2C(sizearr-1,i , sizearr)] = A[IDX2C(0, sizearr-1, sizearr)]+ step*i ;
            A[IDX2C(i, sizearr-1, sizearr)] = A[IDX2C(0, sizearr-1, sizearr)]+ step*i;

            Anew[IDX2C(0, i, sizearr)] = Anew[IDX2C(0, 0, sizearr)] + step*i;
            Anew[IDX2C(i, 0, sizearr)] = Anew[IDX2C(0, 0, sizearr)] + step*i;
	        Anew[IDX2C(sizearr-1,i , sizearr)] = Anew[IDX2C(0, sizearr-1, sizearr)]+ step*i ;
            Anew[IDX2C(i, sizearr-1, sizearr)] = Anew[IDX2C(0, sizearr-1, sizearr)]+ step*i;
        }
}

int main(int argc, char** argv)
{
    
	clock_t start = clock();
    int itermax;
	double tol;

    //получаю значения из параметров командной строки.
	tol = atof(argv[1]);
	sizearr = atof(argv[2]);
	itermax = atof(argv[3]);

    //указываю какую видеокарту использовать
    acc_set_device_num(0, acc_device_default);

	double err = 1;
	int iter = 0;
    double* Aerr;

    //выделяю память под массивы
	Anew = (double*)calloc(sizearr *sizearr, sizeof(double));
	A = (double*)calloc(sizearr *sizearr, sizeof(double));
    Aerr = (double*)calloc(sizearr *sizearr, sizeof(double));

    //вызыавю функцияю для начального заполнения массива 
    completionArr();

    //инициализирую дискриптор
    cublasHandle_t handle;
    //инициализирую переменную статуса
	cublasStatus_t stat;
    //создаю дискриптор
	cublasCreate(&handle);

    int result;
    
    //копирую данные на GPU
    #pragma acc data copyin(Anew[:sizearr*sizearr],A[:sizearr*sizearr],Aerr[:sizearr*sizearr])
    {
        do{
            //
            #pragma acc data present(A,Anew)
            #pragma acc parallel
		    {
            #pragma acc loop independent
            for (int j = 1; j < (sizearr - 1); j++)
            {
                #pragma acc loop independent
                for (int i = 1; i < (sizearr - 1); i++)
		    		{
                        //рассчитываю новое значение ячейки
                        Anew[IDX2C(i, j, sizearr)] = 0.25 * (A[IDX2C(i+1, j, sizearr)] +A[IDX2C(i-1, j, sizearr)]\
                        +A[IDX2C(i, j+1, sizearr)] + A[IDX2C(i, j-1, sizearr)]);
                        //рассчитываю модуль разницы значений новой и старой ячейки
                        Aerr[IDX2C(i, j, sizearr)] =fabs(A[IDX2C(i, j, sizearr)] - Anew[IDX2C(i, j, sizearr)]);
                    }
            }
            }
            if(iter %sizearr == 0)
            {
                #pragma acc host_data use_device(A, Anew, Aerr)
		        {
                    //получаю индекс ячейки с максимальным значением массива Aerr
		            stat = cublasIdamax(handle, sizearr*sizearr, Aerr, 1, &result);
		            if (stat != CUBLAS_STATUS_SUCCESS){
		    	        printf("cublasIdamax failed\n");
		    	        cublasDestroy(handle);
		    	        break;
		            }
                    //получаю значение на CPU ячейки с максимальным значением массива Aerr
                    cublasGetVector(1, sizeof(double), Aerr + result - 1, 1, &err, 1);
                }
            }
            splits();
            iter++;
        }while(iter < itermax && err>tol);
    }
    printf("iter = %d \t err = %lf \n", iter, err);
    
    //освобождаю память
    free(Anew);
    free(A);
    free(Aerr);
    //удаляю дискриптор
    cublasDestroy(handle);

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("The time: %f seconds\n", seconds);
    
    return 0;
}