/*
  Daniel Sá Barretto Prado Garcia 10374344
  Tiago Marino Silva 10734748
  Felipe Guilermmo Santuche Moleiro 10724010
  Laura Genari Alves de Jesus 10801180
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREADS 32
#define INF 0x7fffffff

__global__ void prodEscalar(int* A, int* B, int* somaDosProd, int dim) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;
  if(i < dim && j < dim ){
    int produto = A[i*dim + j] * B[i*dim + j];
    atomicAdd(somaDosProd, produto);
  }
}

__global__ void min_max_elementos(int* A, int* B, int* max_comp, int* min_comp, int dim)
{
    //Calcula o índice global da thread
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i < dim && j < dim)
    {
        //Faz a soma entre elemento do vetor no índice idx e o conteúdo de soma
        int local_max = A[i*dim + j] > B[i*dim + j] ? A[i*dim + j] : B[i*dim + j];
        int local_min = A[i*dim + j] < B[i*dim + j] ? A[i*dim + j] : B[i*dim + j];
        
        atomicMax(max_comp, local_max);
        atomicMin(min_comp, local_min);
    }
}

int main(int argc, char **argv) 
{
  // Declara as matrizes
  int *A, *B;

  // Declara as variáveis de índice
  int i, j, dim;

  FILE *inputfile;      // handler para o arquivo de entrada
  char *inputfilename;  // nome do arquivo de entrada

  if (argc < 2) {
    printf("Please run with input file name, i.e., num_perf_mpi inputfile.ext\n");
    exit(-1);
  }

  inputfilename = (char *)malloc(256 * sizeof(char));
  strcpy(inputfilename, argv[1]);

  if ((inputfile = fopen(inputfilename, "r")) == 0) {
    printf("Error openning input file.\n");
    exit(-1);
  }

  fscanf(inputfile, "%d\n", &dim);  // Lê a dimensão das matrizes

  
  // Aloca as matrizes
  A = (int *)malloc(dim * dim * sizeof(int));
  B = (int *)malloc(dim * dim * sizeof(int));


  // Lê a matriz A
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      fscanf(inputfile, "%d ", &(A[i * dim + j]));
    }
  }

  // Lê a matriz B
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      fscanf(inputfile, "%d ", &(B[i * dim + j]));
    }
  }

  // fecha o arquivo de entrada
  fclose(inputfile);

  int *A_d, *B_d;
  //aloca matrizes na gpu
  cudaMalloc(&A_d,dim * dim * sizeof(int));
  cudaMalloc(&B_d,dim * dim * sizeof(int));

  cudaMemcpy(A_d, A, dim*dim*sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(B_d, B, dim*dim*sizeof(int), cudaMemcpyHostToDevice);
  
  int max = -INF, min = INF, soma = 0;
  int *min_D, *max_D, *soma_D;
  cudaMalloc(&min_D, sizeof(int));
  cudaMalloc(&max_D, sizeof(int));  
  cudaMalloc(&soma_D, sizeof(int));
  
  cudaMemcpy(max_D, &max, sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(min_D, &min, sizeof(int), cudaMemcpyHostToDevice);
  cudaMemcpy(soma_D, &soma, sizeof(int), cudaMemcpyHostToDevice);
  
  //Define a quantidade de threads por bloco
  dim3 threadsPerBlock(THREADS,THREADS);
  //Define a quantidade de blocos por grade
  dim3 blocksPerGrid((dim+(threadsPerBlock.x-1)) / threadsPerBlock.x, (dim+(threadsPerBlock.y-1)) / threadsPerBlock.y);

  min_max_elementos<<<blocksPerGrid, threadsPerBlock>>>(A_d, B_d, max_D, min_D, dim);
  
  prodEscalar<<<blocksPerGrid, threadsPerBlock>>>(A_d, B_d, soma_D, dim);

  
  //Copia o resultado da soma de volta para o host
  //cudaMemcpy(prod_escalar, prod_D, dim * sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(&max, max_D,sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(&min, min_D,sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(&soma, soma_D,sizeof(int), cudaMemcpyDeviceToHost);
 
  //Imprime o resultado
  printf("%d %d %d\n", soma, min, max); 

  // Liberação de memória alocada
  free(A);
  free(B);
  cudaFree(B_d);
  cudaFree(A_d);
  cudaFree(min_D);
  cudaFree(max_D);
  cudaFree(soma_D);
 
}