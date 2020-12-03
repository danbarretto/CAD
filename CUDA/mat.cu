#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREADS 10

// BOTA GLOBAL
__global__ void prodEscalar(int* A, int* B, int* prodEsc, int* somaDosProd, int dim) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;
  if(i < dim && j < dim ){
    int produto = A[i*dim + j] * B[i*dim + j];
    atomicAdd(prodEsc+i, produto); 
    atomicAdd(somaDosProd, produto);
  }
}

__global__ void soma_elementos(int *vetorA,int *soma,int tam){
    //Calcula o índice global da thread
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (idx < tam){
        //Faz a soma entre elemento do vetor no índice idx e o conteúdo de soma
        atomicAdd(soma,vetorA[idx]); 
    }  
}

__global__ void min_max_elementos(int *A, int *B, int *max_comp, int tam)
{
    //Calcula o índice global da thread
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (idx < tam)
    {
        //Faz a soma entre elemento do vetor no índice idx e o conteúdo de soma
        int local_max = A[i*dim + j] > B[i*dim + j] ? A : B;
        int local_min = A[i*dim + j] < B[i*dim + j] ? A : B;
        
        atomicMax(max_comp, local_max);
        atomicMin(max_comp, local_min);
    }
}

// Matriz A e B na memoria global
// 

int main(int argc, char **argv) {
  // Declara as matrizes
  int *A, *B;

  // Declara as variáveis de índice
  int i, j, k, dim;
  // Declara o acumulador para o produto escalar global
  int somape, minimo, maximo;

  // Declara um vetor para os produtos escalares locais
  int *prod_escalar;

  FILE *inputfile;      // handler para o arquivo de entrada
  char *inputfilename;  // nome do arquivo de entrada

  if (argc < 2) {
    printf("Please run with input file name, i.e., num_perf_mpi inputfile.ext\n");
    exit(-1);
  }

  inputfilename = (char *)malloc(256 * sizeof(char));
  strcpy(inputfilename, argv[1]);

  printf("inputfilename=%s\n", inputfilename);
  fflush(0);

  if ((inputfile = fopen(inputfilename, "r")) == 0) {
    printf("Error openning input file.\n");
    exit(-1);
  }

  fscanf(inputfile, "%d\n", &dim);  // Lê a dimensão das matrizes

  
  // Aloca as matrizes
  A = (int *)malloc(dim * dim * sizeof(int));
  B = (int *)malloc(dim * dim * sizeof(int));

  // Aloca um vetor para armazenar os produtos escalares de cada linha
  prod_escalar = (int *)malloc(dim * sizeof(int));

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

  cudaMemcpy(A, A_d, dim*dim*sizeof(int), cudaMemcpyDeviceToHost);
  cudaMemcpy(B, B_d, dim*dim*sizeof(int), cudaMemcpyDeviceToHost);

  int *min_D, *max_D, *prod_D, *soma_D;
  cudaMalloc(&min_D, sizeof(int));
  cudaMalloc(&max_D, sizeof(int));  
  cudaMalloc(&prod_D, sizeof(int));  
  cudaMalloc(&soma_D, sizeof(int));  
  
  //Define a quantidade de threads por bloco
  dim3 threadsPerBlock(THREADS,THREADS);
  //Define a quantidade de blocos por grade
  dim3 blocksPerGrid((dim+(threadsPerBlock.x-1)) / threadsPerBlock.x, (dim+(threadsPerBlock.y-1)) / threadsPerBlock.y);

  min_max_elementos<<blocksPerGridb, threadsPerBlock>>(A_d, B_d, min_D, dim);
  
  prodEscalar<<blocksPerGridb, threadsPerBlock>>(A_d, B_d, prod_D, min_D, soma_D, dim);



  /*
  // Calcula o produto escalar de cada linha
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      prod_escalar[j] += A[i * dim + j] * B[i * dim + j];
    }
  }

  // Acumula os produtos das linhas (faz o produto escalar global)
  somape = 0;
  for (i = 0; i < dim; i++) {
    somape += prod_escalar[i];
  }

  // encontra o mínimo da matriz A
  minimo = A[0];
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      if (A[i * dim + j] < minimo) {
        minimo = A[i * dim + j];
      }
    }
  }
  // encontra o mínimo da matriz B
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      if (B[i * dim + j] < minimo) {
        minimo = B[i * dim + j];
      }
    }
  }

  // encontra o máximo da matriz A
  maximo = A[0];
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      if (A[i * dim + j] > maximo) {
        maximo = A[i * dim + j];
      }
    }
  }
  // encontra o máximo da matriz B
  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      if (B[i * dim + j] > maximo) {
        maximo = B[i * dim + j];
      }
    }
  }
  */

 
 
  // Imprime o resultado
  printf("%d %d %d\n", somape, minimo, maximo);

  // Libera as matrizes
  free(A);
  free(B);
  cudaFree(B_d);
  cudaFree(A_d);
  cudaFree(prod_escalar_d);
  // Libera o vetor
  free(prod_escalar);
}
