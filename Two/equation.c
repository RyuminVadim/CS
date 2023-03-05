#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int sizearr;
float* A;
float* Anew;
float step;



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
	step = 10 / ((float)sizearr - 1);

	{
		A[0] = (float)10;
		A[sizearr - 1] = (float)20;
		A[sizearr * (sizearr - 1)] = (float)20;
		A[sizearr * sizearr - 1] = 30;

		Anew[0] = (float)10;
		Anew[sizearr - 1] = (float)20;
		Anew[sizearr * (sizearr - 1)] = (float)20;
		Anew[sizearr * sizearr - 1] = 30;


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
}

int main(int argc, char** argv)
{
	//float tol = atof(argv[1]);
	//sizearr = atof(argv[2]);
	//size_t itermax = atof(argv[3]);

	sizearr = 128;
	int itermax = 1000000;
	float tol = 0.000006;
	float err = 30;
	int iter = 0;

	Anew = (float*)calloc(sizearr * sizearr, sizeof(float));
	A = (float*)calloc(sizearr * sizearr, sizeof(float));

	completionArr();
	splits();

#pragma acc data copy(err,iter) create(Anew[:sizearr * sizearr], A[:sizearr * sizearr],step) copyin(itermax,tol,sizearr)
	{

		while (iter < itermax && err>tol) {
			err = 0;
			iter++;


			//#pragma acc parallel reduction(max:err)
#pragma acc data present(Anew, A)
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


			splits();
			//printf("iter = %zu \t err = %f \n", iter, err);


		}
	}

	//printf("size_arr = %zu * %zu \t iter_max= %zu \t err = %f \n", sizearr, sizearr, itermax, tol);
	printf("iter = %zu \t err = %f \n", iter, err);

	free(A);
	free(Anew);
	return 0;
}