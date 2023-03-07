#include <stdio.h>
#include <math.h>
#include <stdlib.h>

//#include <nvToolsExt.h>

int sizearr;
float** A;
float** Anew;

void printArr(float** arr) {
	for (int i = 0; i < sizearr; i++)
	{
		for (int j = 0; j < sizearr; j++)
		{
			printf("%f ", arr[i][j]);
		}
		printf("\n");
	}
}

void splits() {
	float** split = A;
	A = Anew;
	Anew = split;

}

void completionArr() {
	for (int i = 0; i < sizearr; i++)
	{
		Anew[i] = (float*)calloc(sizearr, sizeof(float));
		A[i] = (float*)calloc(sizearr, sizeof(float));
	}

#pragma acc data present(Anew, A,sizearr)
	float step = 10 / ((float)sizearr - 1);
#pragma acc data copyin(step)
	{
#pragma acc loop independent
		{
			A[0][0] = 10.0;
			A[sizearr - 1][0] = 20.0;
			A[0][sizearr - 1] = 20.0;
			A[sizearr - 1][sizearr - 1] = 30.0;

			Anew[0][0] = 10.0;
			Anew[sizearr - 1][0] = 20.0;
			Anew[0][sizearr - 1] = 20.0;
			Anew[sizearr - 1][sizearr - 1] = 30.0;
		}

#pragma acc data present(Anew, A,sizearr)
#pragma acc parallel loop independent
		for (int i = 1; i < sizearr; i++)
		{
			A[0][i] = A[0][0] + step * i;
			Anew[0][i] = Anew[0][0] + step * i;

			A[i][0] = A[0][0] + step * i;
			Anew[i][0] = Anew[0][0] + step * i;

			A[sizearr - 1][i] = A[sizearr - 1][0] + step * i;
			Anew[sizearr - 1][i] = Anew[sizearr - 1][0] + step * i;

			A[i][sizearr - 1] = A[0][sizearr - 1] + step * i;
			Anew[i][sizearr - 1] = Anew[0][sizearr - 1] + step * i;
		}
	}
}


int main(int argc, char** argv)
{

	int itermax;
	float tol;
	tol = atof(argv[1]);
	sizearr = atof(argv[2]);
	itermax = atof(argv[3]);
	//sizearr = 8;
	//itermax = 1;
	//tol = 0.000006;
	float err;
	int iter = 0;

	Anew = (float**)malloc(sizearr * sizeof(float*));
	A = (float**)malloc(sizearr * sizeof(float*));



#pragma acc data copyin(Anew[:sizearr][: sizearr], A[:sizearr ][: sizearr],sizearr) create(err)
	{
		completionArr();
		//splits();


		do {
			err = 0;
			iter++;
#pragma acc update device (err)
#pragma acc data present(Anew, A,sizearr)
#pragma acc parallel reduction(max:err)
			{
#pragma acc loop independent
				for (int i = 1; i < (sizearr - 1); i++)
				{
#pragma acc loop independent
					for (int j = 1; j < (sizearr - 1); j++)
					{
						//Anew[i][j] = i;
						Anew[i][j] = 0.25 * (A[i - 1][j] + A[i + 1][j] + A[i][j - 1] + A[i][j + 1]);
						err = fmax(Anew[i][j] - A[i][j], err);
					}

				}
			}
#pragma acc data present(err)
#pragma acc update host (err)
			splits();
		} while (iter < itermax && err>tol);

	}
	//nvtxRangeEnd(id);
	printf("iter = %zu \t err = %f \n", iter, err);

	printArr(A);

	for (int i = 0; i < sizearr; i++)
	{
		free(A[i]);
		free(Anew[i]);
	}
	free(A);
	free(Anew);

	return 0;
}