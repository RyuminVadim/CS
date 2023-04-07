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


void splits() {
	double* split = A;
	A = Anew;
	Anew = split;
}

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
    
	
    int itermax;
	double tol;

	tol = atof(argv[1]);
	sizearr = atof(argv[2]);
	itermax = atof(argv[3]);
    acc_set_device_num(0, acc_device_default);

	double err = 1;
	int iter = 0;
    double* Aerr;

	Anew = (double*)calloc(sizearr *sizearr, sizeof(double));
	A = (double*)calloc(sizearr *sizearr, sizeof(double));
    Aerr = (double*)calloc(sizearr *sizearr, sizeof(double));

    completionArr();

    cublasHandle_t handle;
	cublasStatus_t stat;
	cublasCreate(&handle);

    int result;
    clock_t start = clock();

    #pragma acc data copyin(Anew[:sizearr*sizearr],A[:sizearr*sizearr],Aerr[:sizearr*sizearr])
    {
        do{
            #pragma acc data present(A,Anew)
            #pragma acc parallel
		    {
            #pragma acc loop independent
            for (int j = 1; j < (sizearr - 1); j++)
            {
                #pragma acc loop independent
                for (int i = 1; i < (sizearr - 1); i++)
		    		{
                        Anew[IDX2C(i, j, sizearr)] = 0.25 * (A[IDX2C(i+1, j, sizearr)] +A[IDX2C(i-1, j, sizearr)]\
                        +A[IDX2C(i, j+1, sizearr)] + A[IDX2C(i, j-1, sizearr)]);
                        Aerr[IDX2C(i, j, sizearr)] =fabs(A[IDX2C(i, j, sizearr)] - Anew[IDX2C(i, j, sizearr)]);
                    }
            }
            }
            if(iter %sizearr == 0)
            {
                #pragma acc host_data use_device(A, Anew, Aerr)
		        {
                    
		            stat = cublasIdamax(handle, sizearr*sizearr, Aerr, 1, &result);
		            if (stat != CUBLAS_STATUS_SUCCESS){
		    	        printf("cublasIdamax failed\n");
		    	        cublasDestroy(handle);
		    	        break;
		            }
                    cublasGetVector(1, sizeof(double), Aerr + result - 1, 1, &err, 1);
                }
            }
            splits();
            iter++;
        }while(iter < itermax && err>tol);
    }
    printf("iter = %d \t err = %lf \n", iter, err);
    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("The time: %f seconds\n", seconds);
 
    free(Anew);
    free(A);
    free(Aerr);
    cublasDestroy(handle);

    
    return 0;
}