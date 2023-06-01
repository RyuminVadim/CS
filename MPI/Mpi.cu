#include <iostream>
#include <cstring>
#include <sstream>
#include <cmath>
#include <ctime>
#include <iomanip>

#include <cuda_runtime.h>
#include <cub/cub.cuh>

#include "mpi.h"

#define UPDATE size
#define THREADS_MAX 1024
#define THREAD (size < THREADS_MAX ? size : THREADS_MAX)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Функция изменения матрицы
__global__ void iterate(double* A, double* A_new, size_t size_x, size_t size_y) {
	size_t j = blockIdx.x * blockDim.x + threadIdx.x;
	size_t i = blockIdx.y * blockDim.y + threadIdx.y;
	
	if ((j > 0) && (i > 0) && (i < size_y - 1) && (j < size_x - 1))  // Don't update borders
		A_new[i * size_x + j] = 0.25 * (A[i * size_x + j - 1] + A[(i - 1) * size_x + j] + A[(i + 1) * size_x + j] + A[i * size_x + j + 1]);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Функция разницы матриц
__global__ void subtraction(double* A, double* A_new, double* A_err, size_t size_x) {
	int j = blockIdx.x * blockDim.x + threadIdx.x;
	int i = blockIdx.y * blockDim.y + threadIdx.y;
	if ((j > 0) && (i > 0) && (i < size_x - 1) && (j < size_x - 1))
		A_err[i * size_x + j] = A[i * size_x + j] - A_new[i * size_x + j];
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Значения по умодчанию
double eps = 1E-6;
int size = 256;
int iter_max = 1E6;

int main(int argc, char** argv) {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Получение значений из командной строки
	sscanf(argv[1], "%lf", &eps);
	sscanf(argv[2], "%d", &size);
	sscanf(argv[3], "%d", &iter_max);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Выбор видеокарт
	int DEVICE, COUNT_DEVICE;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &DEVICE);
	MPI_Comm_size(MPI_COMM_WORLD, &COUNT_DEVICE);

	cudaSetDevice(DEVICE);

	std::cout << "Number of processes: " << COUNT_DEVICE << std::endl;
	if (DEVICE == 0)
		std::cout << "Settings: " << "\n\tMin error: " << eps << "\n\tMax iteration: " << iter_max << "\n\tSize: " << size << "x" << size << std::endl;

	size_t size_y; 
	
	size_y = size / COUNT_DEVICE;
	if (COUNT_DEVICE > 1)
		size_y++;
	if (DEVICE != COUNT_DEVICE - 1 && DEVICE != 0) size_y += 1;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Выделения памяти
	double *A, *A_Device, *A_new_Device, *A_error_Device, *deviceError, *tempStorage = NULL;
	size_t tempStorageSize = 0;

	cudaMallocHost(&A, sizeof(double) * size * size);
	for (int i = 0; i < size; i++)  {
		A[i] = 10.0 + i * 10.0 / (size - 1);
		A[i * size] = 10.0 + i * 10.0 / (size - 1);
		A[size - 1 + i * size] = 20.0 + i * 10.0 / (size - 1);
		A[size * (size - 1) + i] = 20.0 + i * 10.0 / (size - 1);
	}

	dim3 threads(THREAD,1);
	dim3 blocks(size/THREAD, size_y);

	cudaMalloc(&A_Device, sizeof(double) * size * size_y);
	cudaMalloc(&A_new_Device, sizeof(double) * size * size_y);
	cudaMalloc(&A_error_Device, sizeof(double) * size * size_y);
	cudaMalloc(&deviceError, sizeof(double));

	size_t offset = (DEVICE != 0) ? size : 0;
 	cudaMemcpy(A_Device, A + (size * size * DEVICE / COUNT_DEVICE) - offset, sizeof(double) * size * size_y, cudaMemcpyHostToDevice);
	cudaMemcpy(A_new_Device, A + (size * size * DEVICE / COUNT_DEVICE) - offset, sizeof(double) * size * size_y, cudaMemcpyHostToDevice);

	cub::DeviceReduce::Max(tempStorage, tempStorageSize, A_error_Device, deviceError, size * size_y);
	cudaMalloc(&tempStorage, tempStorageSize);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Основной цикл
	size_t iter = 0;
	double error = 1.0;
	clock_t begin = clock();
	cudaStream_t stream, datatransfer;
	cudaStreamCreate(&stream);
	cudaStreamCreate(&datatransfer);
	while((iter < iter_max) && error > eps)	{
		iterate<<<blocks, threads, 0, stream>>>(A_Device, A_new_Device, size, size_y);
		iter++;
		// Расчитываем ошибку каждую итерацию кратную размеру матрицы
		if (iter % UPDATE == 0 ) {
			
			subtraction<<<blocks, threads, 0, stream>>>(A_new_Device, A_Device, A_error_Device, size);
			cub::DeviceReduce::Max(tempStorage, tempStorageSize, A_error_Device, deviceError, size * size_y, stream);
			cudaStreamSynchronize(stream);

			if (COUNT_DEVICE > 1) {
			cudaMemcpyAsync(&error, deviceError, sizeof(double), cudaMemcpyDeviceToHost, stream);
				//передаём ошибку всем процессам
				MPI_Allreduce((void*)&error, (void*)&error, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
			}
			else
				cudaMemcpy(&error, deviceError, sizeof(double), cudaMemcpyDeviceToHost);

		}

		if (DEVICE > 0)			   // Обмен верхней границей
			MPI_Sendrecv(A_new_Device + size + 1, size - 2, MPI_DOUBLE, DEVICE - 1, 0, A_new_Device + 1, size - 2, MPI_DOUBLE, DEVICE - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		if (DEVICE < COUNT_DEVICE - 1) // Обмен нижней границей
			MPI_Sendrecv(A_new_Device + (size_y - 2) * size + 1, size - 2, MPI_DOUBLE, DEVICE + 1, 0, A_new_Device + (size_y - 1) * size + 1, size - 2, MPI_DOUBLE, DEVICE + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


		std::swap(A_Device, A_new_Device);
	}

	clock_t end = clock();
	if (DEVICE == 0) {
		std::cout << "Result:\n\tIter: " << iter << "\n\tError: " << error << "\n\tTime: " << 1.0 * (end - begin) / CLOCKS_PER_SEC << std::endl;
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////// Чистка памяти
	cudaFree(A_Device);
	cudaFree(A_new_Device);
	cudaFree(A_error_Device);
	cudaFree(tempStorage);
	cudaStreamDestroy(stream);
	cudaStreamDestroy(datatransfer);
	MPI_Finalize();

	return 0;
}
