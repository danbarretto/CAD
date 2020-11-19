/*
  PCAM


*/
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 10
#define NUMTHREADS 4

int* cria_matriz_vetor(int n, int m) {
  return (int*)calloc(n * m, sizeof(int));
}

int rand_vet(int* vet, int n) {
  for (int i = 0; i < n; i++) {
    vet[i] = rand() % 10;
  }
}

void print_vet_mat(int* vet, int n, int m) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      printf("%d ", vet[i * n + j]);
    }
    printf("\n");
  }
}

void destroy_mat(int** mat, int n) {
  for (int i = 0; i < n; i++) free(mat[i]);

  free(mat);
}

int main(int argc, char* argv[]) {
  int npes, myrank, src, dest, msgtag;
  int token;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  

  int *B = cria_matriz_vetor(N, N), *A, *C;
  if (myrank == 0) {
    A = cria_matriz_vetor(N, N);
    C = cria_matriz_vetor(N, N);
    
    srand(time(NULL));
    rand_vet(A, N * N);
    rand_vet(B, N * N);
  }
  int from = myrank * N / npes;
  int to = (myrank + 1) * N / npes;

  MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

  if (N % npes == 0) {
    // Compute Equally Distributed
    if(myrank != 0) {
      A = cria_matriz_vetor(N / npes, N);
      C = cria_matriz_vetor(N / npes, N);
    }
      
    MPI_Scatter(A, N * N / npes, MPI_INT, A, N * N / npes, MPI_INT, 0, MPI_COMM_WORLD);
    
    // OPENMP
    int i, j, k;
    #pragma omp parallel for private(j, i, k) num_threads(NUMTHREADS)
    for (j = 0; j < N; j++) {
      for (i = 0; i < N / npes; i++) {
        for (k = 0; k < N; k++) {
          C[i * N + j] += A[i * N + k] * B[k * N + j];
        }
      }
    }

    MPI_Gather(C, N * N / npes, MPI_INT, C, N * N / npes, MPI_INT, 0, MPI_COMM_WORLD);
  } 
  else {
    // Compute Not Equally Distributed;
    int* sendcounts = (int*) malloc(sizeof(int) * npes);
    int* displacement = (int*) malloc(sizeof(int) * npes);
    
    // All processes have N * N / npes integers to compute
    for(int i = 0; i < npes; i++) {
      sendcounts[i] = N * N / npes;
    }
    // Last process has the missing part
    sendcounts[npes - 1] += N * (N % npes);

    //Calculate displacement
    for(int i = 0; i < npes; i++) {
      displacement[i] = (N * N / npes) * i;
    }
    // Allocate memory for child processes
    if(myrank != 0) {
      A = cria_matriz_vetor((sendcounts[myrank] / N)+1, N);
      C = cria_matriz_vetor((sendcounts[myrank] / N)+1, N);
    }

    // Scatter data
    MPI_Scatterv(A, sendcounts, displacement, MPI_INT, A, sendcounts[myrank], MPI_INT, 0, MPI_COMM_WORLD);

    // Matrix parallel dot product
    int i, j, k;
    #pragma omp parallel for private(j, i, k) num_threads(NUMTHREADS)
    for (j = 0; j < N; j++) {
      for (i = 0; i < N / npes; i++) {
        for (k = 0; k < N; k++) {
          C[i * N + j] += A[i * N + k] * B[k * N + j];
        }
      }
    }

    // Gather Result
    MPI_Gatherv(C, N * N / npes, MPI_INT, C, sendcounts, displacement, MPI_INT, 0, MPI_COMM_WORLD);
    free(sendcounts);
    free(displacement);
  }

  // Print and destroy matrices
  if (myrank == 0) {
    printf("\n\n");
    print_vet_mat(A, N, N);
    printf("\n\n\t       * \n");
    print_vet_mat(B, N, N);
    printf("\n\n\t       = \n");
    print_vet_mat(C, N, N);
    printf("\n\n");

    fflush(0);
  }
  
  free(A);
  free(B);
  free(C);

  return !(MPI_Finalize() == MPI_SUCCESS);
}