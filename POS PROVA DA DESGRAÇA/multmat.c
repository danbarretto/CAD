/*
  PCAM


*/
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

  //matriz B esta transposta!!
  int *B = cria_matriz_vetor(N, N), *A, *C,*disp,*sizes;

  int rec_size= N/npes;
  int rec_size_resto=rec_size+(N%npes);

  if (myrank == 0) {
    A = cria_matriz_vetor(N, N);
    C = cria_matriz_vetor(N, N);
    rand_vet(A, N * N);
    rand_vet(B, N * N);
    disp = (int*)malloc(npes*sizeof(int));
    sizes = (int*)malloc(npes*sizeof(int));

    for(int i=0;i<npes;i++){
      disp[i] = i*(rec_size*N);
      sizes[i] = rec_size*N;
      //if(sizes[i] == 0) sizes[i] = 1;
    }
    sizes[npes-1] = rec_size_resto*N;

  } else {
    A = cria_matriz_vetor(rec_size_resto, N);
    C = cria_matriz_vetor(rec_size_resto, N);
  }

 
  MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
  
  if(myrank == 0){
    for(int i=0;i<npes;i++){
      printf("i: %d, sizes: %d, disp: %d\n",i, sizes[i],disp[i]);
    }
  }

  int num_linhas;
  if(myrank == npes-1) num_linhas = rec_size_resto;
  else num_linhas = rec_size;

  //esse if existe pois o MPI Scatterv da erro se existir alguem recebendo informação de tamanho 0, por algum motivo o processo que recebe
  //o buffer de tamanho 0 fica esperando informação para sempre ao inves de nao receber nada e continuar, para resolver isso basta fazer so os processos
  //que vao realmente mandar e receber informaçao entrarem nessa função PS: esse caso so acontece qndo N < NPROC, o que é bem raro, mas temos que tratar pois pode acontecer
  if(myrank == 0 || num_linhas != 0){
    MPI_Scatterv (A,sizes,disp,MPI_INT,A,rec_size_resto*N,MPI_INT,0,MPI_COMM_WORLD);
  }
  

  printf("tamo aqui:  %d\n", myrank);
  

  
  // OPENMP
  int i, j, k;
  #pragma omp parallel for private(j, i, k) num_threads(NUMTHREADS)
  for (j = 0; j < N; j++) {
    for (i = 0; i < num_linhas; i++) {
      for (k = 0; k < N; k++) {
        //printf("\nmyrank=%d thread num:%d", myrank, omp_get_thread_num());
        //printf("\nC[%d, %d] += %d * %d\n", i * N, j, A[i * N + k]
        C[i * N + j] += A[i * N + k] * B[j * N + k];//linha de A vezes linha de B(pois B esta transposta)
      }
    }
  }

  

    print_vet_mat(C, N, N, 0);
    printf("\n\n");

  MPI_Gatherv( C,N*rec_size_resto,MPI_INT,C,sizes,disp,MPI_INT, 0, MPI_COMM_WORLD);
  
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
  free(A);
  free(B);
  free(C);
  if (myrank == 0){
    free(disp);
    free(sizes);
  } 

  return !(MPI_Finalize() == MPI_SUCCESS);
}
