#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


size_t sizearr =0;
float** A;
float** Anew;

void completion_arr_t() {
	float step = 10 / ((float)sizearr - 1);
	printf("%f\n", step);
	A[0][0] = 10;
	A[sizearr - 1][0] = 20;
	A[0][sizearr - 1] = 20;
	A[sizearr - 1][sizearr - 1] = 30;
	for (size_t i = 1; i < sizearr - 1; i++)
	{
		A[i][0] = A[0][0] + step * i;
		A[0][i] = A[0][0] + step * i;
		A[sizearr - 1][i] = A[sizearr - 1][0] + step * i;
		A[i][sizearr - 1] = A[0][sizearr - 1] + step * i;
	}

}

void completion_arr() {
	float step = 10 / ((float)sizearr - 1);

	A[0][0] = 10;
	A[sizearr - 1][0] = 20;
	A[0][sizearr - 1] = 20;
	A[sizearr - 1][sizearr - 1] = 30;
	//распаралелить 
	for (size_t i = 1; i < sizearr - 1; i++)
	{
		A[i][0] = A[i - 1][0] + step;
		A[0][i] = A[0][i - 1] + step;
		A[sizearr - 1][i] = A[sizearr - 1][i - 1] + step;
		A[i][sizearr - 1] = A[i - 1][sizearr - 1] + step;
	}

}

void creating_array() {
	// распаралелить
	for (size_t i = 0; i < sizearr; i++)
	{
		Anew[i] = (float*)malloc(sizearr * sizeof(float));
		A[i] = (float*)malloc(sizearr * sizeof(float));
		A[i] = (float*)calloc(sizearr, sizeof(float));
	}

}


void copy_arr() {
	// распаралелить
	for (size_t i = 0; i < sizearr; i++)
	{
		memcpy(Anew[i], A[i], sizeof(float*) * sizearr / 2);
	}
}

int main(int argc, char** argv)
{

	//float iter = 0;
	//float tol = atof(argv[1]);
	//float sizearr = atof(argv[2]);
	//float itermax = atof(argv[3]);

	sizearr = atof(argv[2]);
	size_t itermax = atof(argv[3]);
	float tol = atof(argv[1]);
	float err = 30;
	size_t iter = 0;
	//добавить работу с данными

	A = (float**)malloc(sizearr * sizeof(float*));
	Anew = (float**)malloc(sizearr * sizeof(float*));

	creating_array();
	completion_arr();
	copy_arr();
	//print_arr(arriter);
	float val;
	while (iter < itermax && err>tol) {
		err = 0;
		// распаралелить
		for (size_t i = 1; i < sizearr - 1; i++)
		{
			// распаралелить
			for (size_t j = 1; j < sizearr - 1; j++)
			{
				A[i][j] = (Anew[i + 1][j] + Anew[i - 1][j] + Anew[i][j + 1] + Anew[i][j - 1]) / 4;
				val = abs(A[i][j] - Anew[i][j])
				err = max(err, val);
			}
		}
		copy_arr();
		iter++;
	}

	printf("err =%f, iter = %zu", err, iter);
	// распаралелить
	for (size_t i = 0; i < sizearr; i++)
	{
		free(A[i]);
	}
	free(A);

	// распаралелить
	for (size_t i = 0; i < sizearr; i++)
	{
		free(Anew[i]);
	}
	free(Anew);
	return 0;
}
