#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <nvToolsExt.h>

int sizearr;
float* A;
float* Anew;

void printArr(float* arr) {
	for (int i = 0; i < sizearr * sizearr; i++)
	{
		if (i % sizearr == 0)
			printf("\n");
		printf("%f ", arr[i]);
	}
}

void splits() {
	float* split = A;
	A = Anew;
	Anew = split;

}

void completionArr() {
	float step = 10 / ((float)sizearr - 1);

	A[0] = 10.0;
	A[sizearr - 1] = 20.0;
	A[sizearr * (sizearr - 1)] = 20.0;
	A[sizearr * sizearr - 1] = 30.0;

	Anew[0] = 10.0;
	Anew[sizearr - 1] = 20.0;
	Anew[sizearr * (sizearr - 1)] = 20.0;
	Anew[sizearr * sizearr - 1] = 30.0;

#pragma acc parallel loop
	for (int i = 1; i < sizearr; i++)
	{
		A[i] = A[0] + step * i;
		Anew[i] = Anew[0] + step * i;

		A[i * sizearr] = A[0] + step * i;
		Anew[i * sizearr] = Anew[0] + step * i;

		A[i * sizearr + sizearr - 1] = A[sizearr - 1] + step * i;
		Anew[i * sizearr + sizearr - 1] = Anew[sizearr - 1] + step * i;

		A[(sizearr - 1) * sizearr + i] = A[sizearr - 1] + step * i;
		Anew[(sizearr - 1) * sizearr + i] = Anew[sizearr - 1] + step * i;
	}
}

int main(int argc, char** argv)
{
	int itermax;
	float tol;
	tol = atof(argv[1]);
	sizearr = atof(argv[2]);
	itermax = atof(argv[3]);

	//sizearr = 128;
	//itermax = 1000000;
	//tol = 0.000006;
	float err;
	int iter = 0;

	Anew = (float*)calloc(sizearr * sizearr, sizeof(float));
	A = (float*)calloc(sizearr * sizearr, sizeof(float));
	int id = nvtxRangeStartA("ii");

	completionArr();
	splits();

	//#pragma acc data copy(err,iter) create(Anew[:sizearr * sizearr], A[:sizearr * sizearr],step) copyin(itermax,tol,sizearr)
#pragma acc data copyin(Anew[:sizearr * sizearr], A[:sizearr * sizearr],sizearr)
	//{
		 do{
			err = 0;
			iter++;
#pragma acc data present(Anew, A,sizearr)
#pragma acc parallel reduction(max:err)
			{
#pragma acc loop independent
				for (int i = sizearr; i < (sizearr) * (sizearr - 1); i++)
				{
					if (((i) % sizearr) == 0 || ((i) % sizearr) == 7)

						continue;

					Anew[sizearr * (i / (sizearr)) + ((i) % sizearr)] = 0.25 * (A[sizearr * ((i) / sizearr) + ((i + 1) % sizearr)] +
						A[sizearr * ((i) / sizearr) + ((i - 1) % sizearr)] + A[sizearr * ((i / sizearr) - 1) + ((i) % sizearr)] +
						A[sizearr * ((i / sizearr) + 1) + ((i) % sizearr)]);
					err = fmax(Anew[sizearr * (i / (sizearr)) + ((i) % sizearr)] - A[sizearr * (i / (sizearr)) + ((i) % sizearr)], err);
				}
			}
			splits();
		 } while (iter < itermax && err>tol);
		
	//}
	nvtxRangeEnd(id);
	printf("iter = %zu \t err = %f \n", iter, err);

	free(A);
	free(Anew);
	return 0;
}