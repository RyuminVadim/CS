#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


size_t sizearr = 0;
float** A;
float** Anew;

void completion_arr() {
	float step = 10 / ((float)sizearr - 1);

	A[0][0] = 10;
	A[sizearr - 1][0] = 20;
	A[0][sizearr - 1] = 20;
	A[sizearr - 1][sizearr - 1] = 30;

#pragma acc parallel loop independent
	for (int i = 1; i < sizearr - 1; i++)
	{
		A[i][0] = A[i - 1][0] + step;
		A[0][i] = A[0][i - 1] + step;
		A[sizearr - 1][i] = A[sizearr - 1][i - 1] + step;
		A[i][sizearr - 1] = A[i - 1][sizearr - 1] + step;
	}

}

void creating_array() {
	// распаралелить
	#pragma acc parallel loop independent
	for (int i = 0; i < sizearr; i++)
	{
		Anew[i] = (float*)malloc(sizearr * sizeof(float));
		A[i] = (float*)malloc(sizearr * sizeof(float));
		

	}
	for (int i = 0; i < sizearr; i++)
		A[i] = (float*)calloc(sizearr, sizeof(float));


}


void copy_arr() {
	// распаралелить
#pragma acc parallel
	for (int i = 0; i < sizearr; i++)
	{
		memcpy(Anew[i], A[i], sizeof(float*) * sizearr / 2);
	}
}

int main(int argc, char** argv)
{

	//float tol = atof(argv[1]);
	//sizearr = atof(argv[2]);
	//size_t itermax = atof(argv[3]);


	sizearr = 128;
	size_t itermax = 1000000;
	float tol = 0.000006;
	float err = 30;
	size_t iter = 0;
	//добавить работу с данными

	A = (float**)malloc(sizearr * sizeof(float*));
	Anew = (float**)malloc(sizearr * sizeof(float*));

	creating_array();
	completion_arr();
	copy_arr();

	while (iter < itermax && err>tol) {
		err = 0;
		for (int i = 1; i < sizearr - 1; i++)
		{
			for (int j = 1; j < sizearr - 1; j++)
			{
				A[i][j] = (Anew[i + 1][j] + Anew[i - 1][j] + Anew[i][j + 1] + Anew[i][j - 1]) / 4;
				err = err > fabs(A[i][j] - Anew[i][j]) ? err : fabs(A[i][j] - Anew[i][j]);
			}
		}
		copy_arr();
		iter++;
	}

	printf("err =%f, iter = %zu\n", err, iter);
	// распаралелить
	//#pragma acc parallel
	for (int i = 0; i < sizearr; i++)
	{
		free(A[i]);
	}
	free(A);

	// распаралелить
	//#pragma acc parallel
	for (int i = 0; i < sizearr; i++)
	{
		free(Anew[i]);
	}
	free(Anew);
	return 0;
}
