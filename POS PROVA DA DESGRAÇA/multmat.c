/*
  PCAM


*/
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 2
#define NUMTHREADS 4

int* cria_matriz_vetor(int n, int m) {
  return (int*)calloc(n * m, sizeof(int));
}

int rand_vet(int* vet, int n) {
  for (int i = 0; i < n; i++) {
    vet[i] = rand() % 10;
  }
}

void print_vet_mat(int* vet, int n, int m,int transpor) {
  if(transpor){
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        printf("%d ", vet[j * n  + i]);
      }
      printf("\n");
    }
  }else{
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < m; j++) {
        printf("%d ", vet[i * n + j]);
      }
      printf("\n");
    }
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
    rand_vet(A, N * N);
    rand_vet(B, N * N);
  } else {
    A = cria_matriz_vetor(N / npes, N);
    C = cria_matriz_vetor(N / npes, N);
  }
  // int linhasPerProc = N * N/ npes;
  int from = myrank * N / npes;
  int to = (myrank + 1) * N / npes;
  // for (int i = 0; i < N; i++) MPI_Bcast(B[i], N, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

  // if (N % npes == 0)
  MPI_Scatter(A, N * N / npes, MPI_INT, A, N * N / npes, MPI_INT, 0,
              MPI_COMM_WORLD);

  // else {
  /*int sendcounts[npes];
  memset(&sendcounts, linhasPerProc, sizeof(int));
  sendcounts[npes - 1] = (linhasPerProc + (N % npes);

  int displacement[npes];
  for(int i = 0; i < npes; i++){
    displacement[i] = linhasPerProc * i;
    if(i == npes - 1)
      displacement[i] += N % npes;Vamos deixar para a meia noite de hoje.
￼

  }
  memset(&displacement, linhasPerProc, sizeof(int) * linhasPerProc);

  MPI_Scatterv(A,sendcounts, displacement, MPI_INT, A[from],recvcount, MPI_INT,
0 ,MPI_COMM_WORLD);

  //}
  
*/
  // OPENMP
  int i, j, k;
#pragma omp parallel for private(j, i, k) num_threads(NUMTHREADS)
  for (j = 0; j < N; j++) {
    for (i = 0; i < N / npes; i++) {
      for (k = 0; k < N; k++) {
        //printf("\nmyrank=%d thread num:%d", myrank, omp_get_thread_num());
        //printf("\nC[%d, %d] += %d * %d\n", i * N, j, A[i * N + k],
               B[k * N + j]);
        C[i * N + j] += A[i * N + k] * B[j * N + k];//linha de A vezes linha de B(pois B esta transposta)
      }
    }
  }

  MPI_Gather(C, N * N / npes, MPI_INT, C, N * N / npes, MPI_INT, 0,
             MPI_COMM_WORLD);

  // Print and destroy matrices

  if (myrank == 0) {
    printf("\n\n");
    print_vet_mat(A, N, N, 0);
    printf("\n\n\t       * \n");
    print_vet_mat(B, N, N, 1);
    printf("\n\n\t       = \n");
    print_vet_mat(C, N, N, 0);
    printf("\n\n");

    fflush(0);
  }
  /*
  destroy_mat(A, N);
  destroy_mat(B, N);
  destroy_mat(C, N);
  destroy_mat(linhas_A, N / npes);
  destroy_mat(linhas_C, N / npes);
  */
  free(A);
  free(B);
  free(C);

  return !(MPI_Finalize() == MPI_SUCCESS);
}
