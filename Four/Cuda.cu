#include <iostream>
#include <cstring>
#include <sstream>
#include <cmath>

#include <cuda_runtime.h>
#include <cub/cub.cuh>

void complet(double* A,int size){
	double step = 10 / ((double)size - 1);
	for (int i = 0; i < size; i++)
	{
		A[i] = 10.0 + i * step;
		A[i * size] = 10.0 + i * step;
		A[size - 1 + i * size] = 20.0 + i * step;
		A[size * (size - 1) + i] = 20.0 + i * step;
	}
}

void printArr(double* A,int size){
	for (int i = 0; i < size; i++){
		for (int j = 0; j < size; j++){
		std::cout<< A[i * size+j]<<" ";
		}
		std::cout<<std::endl;
	}
}

__global__ void step(double* A, double* A_new, size_t size) {
	size_t i = blockIdx.x + 1, j = threadIdx.x + 1;
	A_new[i * size + j] = 0.25 * (A[i * size + j - 1] + A[(i - 1) * size + j] /
	 + A[(i + 1) * size + j] + A[i * size + j + 1]);	
	}

__global__ void subtraction(double* A, double* A_new, double* A_err) {
	size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
	A_err[idx] = A[idx] - A_new[idx];
}

int main(int argc, char** argv){
	std::cout<< "Cuda"<<std::endl;
	clock_t start = clock();
    int itermax;
    int size;
	double tol;

	if (argc != 4 || atof(argv[1])>1  || atof(argv[3])<1)
    {
        printf("Недостаточно параметров. Нужно ввести 3 параметра\n");
        printf("1.точность;\n2. размер сетки;\n3. количество итераций\n");
        return 0;
    }

	tol = atof(argv[1]);
	size = atof(argv[2]);
	itermax = atof(argv[3]);
	size_t totalSize = size * size;

	double* A= NULL;
    double* A_D, *A_D_new, *A_D_err, *deviceError, *tempStorage = NULL;
	size_t tempStorageSize = 0;

	A = (double*)calloc(totalSize, sizeof(double));

	cudaMalloc(&A_D, sizeof(double) * totalSize);
	cudaMalloc(&A_D_new, sizeof(double) * totalSize);
	cudaMalloc(&A_D_err, sizeof(double) * totalSize);
	cudaMalloc(&deviceError, sizeof(double) );

	cudaMemcpy(A_D_err, A, sizeof(double) * totalSize, cudaMemcpyHostToDevice);

	complet(A,size);

	cudaMemcpy(A_D, A, sizeof(double) * totalSize, cudaMemcpyHostToDevice);
	cudaMemcpy(A_D_new, A, sizeof(double) * totalSize, cudaMemcpyHostToDevice);

	cudaStream_t stream;
	cudaStreamCreate(&stream);
	cudaGraph_t graph;
	cudaGraphExec_t graph_instance;

	cub::DeviceReduce::Max(tempStorage, tempStorageSize, A_D_err, deviceError, totalSize, stream);
	cudaMalloc(&tempStorage, tempStorageSize);
///////////////////////////////////////////////////////////////////// Создание графа
	cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);

	for (size_t i = 0; i < size; i += 2) {
		step<<<size - 2, size - 2, 0, stream>>>(A_D, A_D_new, size);
		//split(A_D, A_D_new);
		step<<<size - 2, size - 2, 0, stream>>>(A_D_new, A_D, size);
	}

	subtraction<<<size, size, 0, stream>>>(A_D, A_D_new,A_D_err);
	cub::DeviceReduce::Max(tempStorage, tempStorageSize, A_D_err, deviceError, totalSize, stream);

	cudaStreamEndCapture(stream, &graph);
	cudaGraphInstantiate(&graph_instance, graph, NULL, NULL, 0);
///////////////////////////////////////////////////////////////////// Основной цикл
	int iter = 0; 
	double error = 1 ;
	 do{
		cudaGraphLaunch(graph_instance, stream);
		cudaMemcpy(&error, deviceError, sizeof(double), cudaMemcpyDeviceToHost);
		//cudaMemcpyFromSymbol(&error, deviceError, sizeof(double), cudaMemcpyDeviceToHost);
		iter += size;
		//std::cout <<error<< std::endl;
	}while(iter < itermax && error > tol);
	std::cout << "Result:\n\tIter: " << iter << "\n\tError: " << error << std::endl;

	//printArr(A,size);

	free(A);
	cudaFree(A_D);
	cudaFree(A_D_new);
	cudaFree(A_D_err);
	cudaFree(deviceError);
	cudaFree(tempStorage);
	cudaGraphDestroy(graph);
	cudaStreamDestroy(stream);
	
	clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("The time: %f seconds\n", seconds);
	return 0;
}