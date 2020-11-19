/*
  Daniel Sá Barretto Prado Garcia 103734344
  Felipe Guilermmo Santuche Moleiro 10724010
  Laura Alves de Jesus 10801180
  Tiago Marino Silva 10734748
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"

int main(int argc, char *argv[]) {
  int npes, myrank, src, dest, ret;

  int name_len;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if (myrank == 0) {
    // Processo pai
    int n, k;
    scanf("%d %d", &n, &k);
    int *vet = (int *)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) scanf("%d", &vet[i]);

    // processo 0 : 0 ate n/npes
    // processo k: n/npes + k ate (k+1)*n/npes
    for (dest = 1; dest < npes; dest++) {
      int ini = dest * (n / npes), final;
      if (dest != npes - 1)
        final = (dest + 1) * (n / npes);
      else
        final = n;
      int tam = final - ini;
      MPI_Send((void *)&tam, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
      MPI_Send((void *)&vet[k], 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      // Envia apenas parte do vetor após ini
      MPI_Send((void *)(vet + ini), tam, MPI_INT, dest, 2, MPI_COMM_WORLD);
    }

    // salva resposta global
    int tam_resGlobal = 0;
    int *resGlobal = (int *)malloc(sizeof(int) * n);

    // executa a parte destinada ao processo 0
    for (int i = 0; i < (int)(n / npes); i++) {
      if (vet[i] > vet[k]) {
        resGlobal[tam_resGlobal++] = i;
      }
    }

    // recebe as respostas dos filhos e adiciona na resposta global
    for (src = 1; src < npes; src++) {
      int tam_res;
      MPI_Recv(&tam_res, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &status);
      int *res = (int *)malloc(sizeof(int) * tam_res);
      MPI_Recv(res, tam_res, MPI_INT, src, 1, MPI_COMM_WORLD, &status);
      for (int i = 0; i < tam_res; i++) {
        resGlobal[tam_resGlobal++] = src * (n / npes) + res[i];
      }
      free(res);
    }

    // print resposta
    printf("\n%d\n", tam_resGlobal);
    for (int i = 0; i < tam_resGlobal; i++) printf("%d ", resGlobal[i]);
    printf("\n");

    free(resGlobal);
    free(vet);
  } else {
    // Processos filhos
    int n, limite;
    int tam;
    MPI_Recv(&tam, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);  // recebe tamanho do vetor
    int *vet = (int *)malloc(sizeof(int) * tam);
    MPI_Recv(&limite, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);  // recebe valor limite(vet[k])
    MPI_Recv(vet, tam, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);  // recebe sua parte do vetor a ser analisada
    int *respostas = (int *)malloc(sizeof(int) * tam);
    int cnt = 0;
    for (int i = 0; i < tam; i++) {
      if (vet[i] > limite) {
        respostas[cnt++] = i;
      }
    }
    MPI_Send((void *)&cnt, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);  // envia tam do vetor de respostas
    MPI_Send(respostas, cnt, MPI_INT, 0, 1, MPI_COMM_WORLD);  // envia vetor de respostas
    free(respostas);
    free(vet);
  }

  fflush(0);
  ret = MPI_Finalize();
  if (ret != MPI_SUCCESS) return 1;
  return (0);
}