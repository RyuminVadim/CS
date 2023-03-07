#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h> 

//#include <nvToolsExt.h>

int sizearr;
float* A;
float* Anew;

void printArr(float* arr) {
	for (int i = 0; i < sizearr * sizearr; i++)
	{
		if (i % sizearr == 0)
			printf("\n");
		printf("%2.3f\t", arr[i]);
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
	double time_spent = 0.0;
	clock_t begin = clock();

	int itermax;
	float tol;
	tol = atof(argv[1]);
	sizearr = atof(argv[2]);
	itermax = atof(argv[3]);

	//sizearr = 128;
	//itermax = 10000000;
	//tol = 0.000001;
	float err;
	int iter = 0;

	Anew = (float*)calloc(sizearr * sizearr, sizeof(float));
	A = (float*)calloc(sizearr * sizearr, sizeof(float));


	completionArr();
	splits();

	do {
		err = 0;
		iter++;
		{
			for (int i = sizearr; i < (sizearr) * (sizearr - 1); i++)
			{
				if (((i) % sizearr) != 0 || ((i) % sizearr) != 7)
				{
					Anew[sizearr * (i / (sizearr)) + ((i) % sizearr)] = 0.25 * (A[sizearr * ((i) / sizearr) + ((i + 1) % sizearr)] +
						A[sizearr * ((i) / sizearr) + ((i - 1) % sizearr)] + A[sizearr * ((i / sizearr) - 1) + ((i) % sizearr)] +
						A[sizearr * ((i / sizearr) + 1) + ((i) % sizearr)]);
					err = fmax(Anew[sizearr * (i / (sizearr)) + ((i) % sizearr)] - A[sizearr * (i / (sizearr)) + ((i) % sizearr)], err);
				}
				else
					continue;
			}
		}

		splits();
	} while (iter < itermax && err>tol);

	printf("iter = %zu \t err = %.8f \n", iter, err);

	//printArr(A);

	free(A);
	free(Anew);

	clock_t end = clock();
	time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
	printf("The elapsed time is %lf seconds", time_spent);


	return 0;
}