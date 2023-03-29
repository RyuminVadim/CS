#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int sizearr;
float* A;
float* Anew;

void splits() {
	float* split = A;
	A = Anew;
	Anew = split;
}

void printArr(float* arr) {
	for (int i = 0; i < sizearr; i++)
	{
		for (int j = 0; j < sizearr; j++)
		{
			printf("%.2f\t", arr[i*sizearr + j]);
		}
		printf("\n");
	}
}


void completionArr() {
    float step = 10 / ((float)sizearr - 1);

    A[0*sizearr + 0] = 10.0;
	A[(sizearr - 1)*sizearr + 0] = 20.0;
	A[0*sizearr + (sizearr - 1)] = 20.0;
	A[(sizearr - 1)*sizearr+(sizearr - 1)] = 30.0;

    Anew[0*sizearr + 0] = 10.0;
	Anew[(sizearr - 1)*sizearr + 0] = 20.0;
	Anew[0*sizearr + (sizearr - 1)] = 20.0;
	Anew[(sizearr - 1)*sizearr+(sizearr - 1)] = 30.0;

    for (int i = 1; i < sizearr; i++)
		{
            A[0*sizearr + i] = A[0*sizearr + 0] +(step * i);
            A[i*sizearr + 0] = A[0*sizearr + 0] +(step * i);
            A[(sizearr-1)*sizearr + i] = A[(sizearr-1)*sizearr + 0] +(step * i);
            A[i*sizearr + (sizearr-1)] = A[(sizearr-1)*sizearr + 0] +(step * i);

            Anew[0*sizearr + i] = A[0*sizearr + 0] +(step * i);
            Anew[i*sizearr + 0] = A[0*sizearr + 0] +(step * i);
            Anew[(sizearr-1)*sizearr + i] = A[(sizearr-1)*sizearr + 0] +(step * i);
            Anew[i*sizearr + (sizearr-1)] = A[(sizearr-1)*sizearr + 0] +(step * i);
        }

}

int main(int argc, char** argv)
{

	int itermax;
	float tol;

	tol = atof(argv[1]);
	sizearr = atof(argv[2]);
	itermax = atof(argv[3]);

    //tol = 0.000001;
	//sizearr = 10;
	//itermax = 100;

	float err;
	int iter = 0;
    int itergpu =0;

	Anew = (float*)calloc(sizearr *sizearr, sizeof(float));
	A = (float*)calloc(sizearr *sizearr, sizeof(float));

    completionArr();
    //printArr(A);
    #pragma acc data copyin(Anew[:sizearr*sizearr],A[:sizearr*sizearr]) 
    do{
        err = 0;
		iter++;
        #pragma acc data present(A,Anew)
        #pragma acc parallel reduction(max:err)
			{
                #pragma acc loop independent
                for (int i = 1; i < (sizearr - 1); i++)
                {
                    #pragma acc loop independent
                    for (int j = 1; j < (sizearr - 1); j++)
		        		{
                            Anew[i*(sizearr)+j] = 0.25 * (A[(i+1)*(sizearr)+(j)]+A[(i-1)*(sizearr)+(j)]\
                            +A[(i)*(sizearr)+(j-1)]+A[(i)*(sizearr)+(j+1)]);
                            err = fmax(err,fabs(A[i*(sizearr)+j]-Anew[i*(sizearr)+j]));
                        }
                }
            }
            splits();

            //#pragma acc update device(err)
           
    }while(iter < itermax && err>tol);

    printf("iter = %d \t err = %f \n", iter, err);
    //printArr(A);

    free(Anew);
    free(A);

    return 0;
}