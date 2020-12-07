/*
Membros:
Daniel Sá Barretto Prado Garcia 103734344
Felipe Guilermmo Santuche Moleiro 10724010
Laura Alves de Jesus 10801180
Tiago Marino Silva 10734748

PCAM:

===Particionamento===
O particionamento será feito da seguinte forma, cada tarefa consiste em uma
multiplicação de uma posição da matriz A por um elemento da matriz B. Ou seja,
dada uma linha da matriz A e uma coluna da matriz B, serão geradas N
tarefas(multiplicam posição i da linha A por i da coluna de B) e após o calculo
dessas multiplicaçoes é feita uma redução de soma dos resultados de cada uma
dessas tarefas para salvar na matriz C. Executa isso para cada permutação de
linhas e colunas possiveis, ou seja, para cada posição da matriz C.

===Comunicação===

Cada tarefa recebe posicao i de uma linha de A e de uma coluna de B, portanto A
e B ou devem ser globais ou devem ter seus valores enviados para cada tarefa.
Aṕos isso se calcula a tarefa, e é necesaria a comunicação com outras tarefas
com outros indices( mas sobre a mesma linha e coluna) para fazer a reducao. Por
ultimo é necessario guardar o resultado na matriz C (global ou enviar resultado
para mestre).

===Aglomeração===

Estamos trabalhando com um cluster com memoria distribuida e em que cada nó do
cluster temos uma maquina MIMD com memoria compartilhada. Nossa aglomeração é
feita da seguinte forma, iremos gerar P processos em termo do numero de nós
existentes no cluster, e iremos dividir a matriz A por P, ou seja, cada nó ira
receber N/P linhas da matriz A e a matriz B inteira. Em cada nó do cluster será
feita a divisão da matriz B por T, em que T é o numero de threads criadas, ou
seja, cada thread do nó vai cuidar de fazer as multiplicaçoes das N/P linhas
pelas N/T colunas, e após a execução de todas as threads, retorna o resultado
para o nó mestre.

===Mapeamento===

Os P processos são mapeados nos nós do cluster, espera-se que esse valor P seja
igual ao numero de nós do cluster para melhor eficiencia, mas caso contrario,
podemos utilizar algoritimos como por exemplo a fila circular para mapear os
processos nos nós. Ja para cada processo iremos criar T threads, esse valor de T
deve ser escolhido pensando no numero de unidades de processamento que existem
no cluster, se todos os nós em to mesmo numero de unidades de processamento,
esse valor é decidido facilmente, ja se temos valores diferentes, o melhor é
escolher baseado no nó com o maior numero de unidades de processamento, dessa
forma não ficamos com nenhuma unidade ociosa. O escalonamento das threads em
cada nó é dado pelo sistema operacional.

*/
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1000
#define NUMTHREADS 4

int* cria_matriz_vetor(int n, int m) {
  return (int*)calloc(n * m, sizeof(int));
}

int rand_vet(int* vet, int n) {
  for (int i = 0; i < n; i++) {
    vet[i] = rand() % 10;
  }
}

void print_vet_mat(int* vet, int n, int m, int transpor) {
  if (transpor) {
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        printf("%d ", vet[j * m + i]);
      }
      printf("\n");
    }
  } else {
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < m; j++) {
        printf("%d ", vet[i * m + j]);
      }
      printf("\n");
    }
  }
}


int main(int argc, char* argv[]) {
  int npes, myrank, src, dest, msgtag;
  int token;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);


  // matriz B esta transposta!!
  int *B = cria_matriz_vetor(N, N), *A, *C;
  int *sendcounts, *displacement;

  int rec_size = N / npes;
  int rec_size_resto = rec_size + (N % npes);

  sendcounts = (int*)malloc(sizeof(int) * npes);
  displacement = (int*)malloc(sizeof(int) * npes);

  // All processes have N * N / npes integers to compute
  // Calculate displacement
  for (int i = 0; i < npes; i++) {
    sendcounts[i] = (rec_size * N);
    displacement[i] = (rec_size * N) * i;
  }
  // Last process has the missing part
  sendcounts[npes - 1] = rec_size_resto * N;

  if (myrank == 0) {
    A = cria_matriz_vetor(N, N);
    C = cria_matriz_vetor(N, N);

    srand(time(NULL));
    rand_vet(A, N * N);
    rand_vet(B, N * N);

  } else {
    // Allocate memory for child processes
    A = cria_matriz_vetor(sendcounts[myrank], N);
    C = cria_matriz_vetor(sendcounts[myrank], N);
  }

  MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

  int num_linhas = myrank == npes - 1 ? rec_size_resto : rec_size;

  // esse if existe pois o MPI Scatterv da erro se existir alguem recebendo
  // informação de tamanho 0, por algum motivo o processo que recebe o buffer de
  // tamanho 0 fica esperando informação para sempre ao inves de nao receber
  // nada e continuar, para resolver isso basta fazer so os processos que vao
  // realmente mandar e receber informaçao entrarem nessa função PS: esse caso
  // so acontece qndo N < NPROC, o que é bem raro, mas temos que tratar pois
  // pode acontecer
  if (myrank == 0 || num_linhas != 0) {
    // Scatter data

    MPI_Scatterv(A, sendcounts, displacement, MPI_INT, A, num_linhas*N,
                 MPI_INT, 0, MPI_COMM_WORLD);
  }
/*
  // printf("\n%d\n", myrank);
  print_vet_mat(A, num_linhas, N, 0);
  MPI_Barrier(MPI_COMM_WORLD);*/
  // Matrix parallel dot product
  int i, j, k;
#pragma omp parallel for private(j, i, k) num_threads(NUMTHREADS)
  for (j = 0; j < N; j++) {
    for (i = 0; i < num_linhas; i++) {
      for (k = 0; k < N; k++) {
        C[i * N + j] += A[i * N + k] * B[j * N + k];
      }
    }
  }
 
  // Gather Result
  MPI_Gatherv(C, N * num_linhas, MPI_INT, C, sendcounts, displacement,
              MPI_INT, 0, MPI_COMM_WORLD);

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

  free(sendcounts);
  free(displacement);
  free(A);
  free(B);
  free(C);

  return !(MPI_Finalize() == MPI_SUCCESS);
}