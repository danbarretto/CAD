#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *cria_matriz_vetor(int n, int m) 
{
  return (int *)calloc(n * m, sizeof(int));
}

void print_vet_mat(int *vet, int n, int m, int transpor) 
{
  if (transpor) 
  {
    for (int i = 0; i < m; i++) 
    {
      for (int j = 0; j < n; j++) 
      {
        printf("%d ", vet[j * m + i]);
      }
      printf("\n");
    }
  } 
  else 
  {
    for (int i = 0; i < n; i++) 
    {
      for (int j = 0; j < m; j++) 
      {
        printf("%d ", vet[i * m + j]);
      }
      printf("\n");
    }
  }
}

int main(int argc, char *argv[]) 
{

  // Declara a matriz MAT e o vetor vet_menor
  int *MAT, *vet_menor;

  // Declara as variáveis de índice
  int i, j, dim;

  // Declara a media para MAT
  double media_MAT = 0;

  //============= MPI ==================//
  int npes, myrank, src, dest, msgtag;
  int token;
  MPI_Status status;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if(myrank == 0) 
  {
    FILE *inputfile;      // handler para o arquivo de entrada
    char *inputfilename;  // nome do arquivo de entrada

    if (argc < 2) 
    {
      printf(
          "Nr errado de argumentos. Execute passando <arq_entrada> como "
          "argumento. \n");
      exit(-1);
    }

    inputfilename = (char *)malloc(256 * sizeof(char));
    strcpy(inputfilename, argv[1]);
    if ((inputfile = fopen(inputfilename, "r")) == 0) 
    {
      printf("Mestre: Erro ao abrir arquivo de entrada %s. Saindo. \n",
            inputfilename);
      exit(-1);
    }

    fscanf(inputfile, "%d\n", &dim);  // Lê a dimensão de MAT

    // Aloca a matriz
    MAT = cria_matriz_vetor(dim, dim);

    // Aloca um vetor para armazenar a quantide de nrs menores que a media por
    // coluna
    vet_menor = (int *)calloc(dim, sizeof(int));

    // Lê a matriz MAT
    for (i = 0; i < dim; i++) 
    {
      for (j = 0; j < dim; j++) 
      {
        fscanf(inputfile, "%d ", &(MAT[j * dim + i]));
      }
    }

    // fecha o arquivo de entrada
    fclose(inputfile);
    free(inputfilename);
  }
  
  MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  int rec_size = dim / npes;
  int rec_size_resto = rec_size + (dim % npes);

  int *sendcounts = (int*)malloc(sizeof(int) * npes);
  int *displacement = (int*)malloc(sizeof(int) * npes);

  // All processes have N * N / npes integers to compute
  // Calculate displacement
  for (int i = 0; i < npes; i++) {
    sendcounts[i] = (rec_size * dim);
    displacement[i] = (rec_size * dim) * i;
  }
  // Last process has the missing part
  sendcounts[npes - 1] = rec_size_resto * dim;

  if(myrank != 0)
  {
    MAT = cria_matriz_vetor(sendcounts[myrank], 1);
  }
  int num_linhas = myrank == npes - 1 ? rec_size_resto : rec_size;

  
  if (myrank == 0 || num_linhas != 0) 
  {
    MPI_Scatterv(MAT, sendcounts, displacement, MPI_INT, MAT, num_linhas*dim,
                 MPI_INT, 0, MPI_COMM_WORLD);
  }

  long long soma = 0;
  long long localSum = 0;
  #pragma omp parallel for private(i,j, localSum)  reduction(+: soma)
  for (j = 0; j < dim; j++) 
  {
    localSum = 0;
    for(i = 0; i < num_linhas; i++)
    {
      localSum += (long long)MAT[i * dim + j];
    }
    soma += localSum;
  }
  
  long long* somatorio;
  if(myrank == 0)  
    somatorio = (long long *)malloc(sizeof(long long)*npes);

  MPI_Gather(&soma, 1, MPI_LONG_LONG_INT, somatorio, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
  
  if(myrank == 0)
  {
    media_MAT = 0;
    for(int i = 0; i < npes; i++)
      media_MAT += somatorio[i];
    media_MAT /= (dim*dim);
  }
  
  
  MPI_Bcast(&media_MAT, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
  if(myrank != 0)
  {
    vet_menor = cria_matriz_vetor(num_linhas, 1);
  }

  // Código com reduction de array funciona a partir do OpenMp 4.5, porém a versão do iguana não suporta
  // Este codigo é mais eficiente para casos em que temos N da matriz menor que npes*num_threads
  // Para casos em que temos N maior que npes*num_threads, não existe diferença consideravel, como a especificação diz que N é bem maior nao teremos problemas
  /*
  if(num_linhas != 0)
  {
    #pragma omp parallel for private(i, j) reduction(+: vet_menor[:num_linhas])
    for (j = 0; j < dim; j++) 
    {
      for(i = 0; i < num_linhas; i++)
      {
        if(MAT[i*dim + j] < media_MAT){
            vet_menor[i] += 1;        
        }
      }
    }
  }
  */

  //Cálculo de elementos menores que a média
  #pragma omp parallel for private(i, j) shared(vet_menor)
  for(i = 0; i < num_linhas; i++)
  {
  for (j = 0; j < dim; j++) 
    {
      if(MAT[i*dim + j] < media_MAT){
          vet_menor[i] += 1;
      }
    }
  }

  //Cálculo de displacement e sendcounts para o vet_menor
  if(myrank ==0){
    for(int i=0;i<npes;i++)
    {
      sendcounts[i] = rec_size;
      displacement[i] = rec_size * i;
    }
    sendcounts[npes-1] = rec_size_resto;
  }
  
  MPI_Gatherv(vet_menor, num_linhas, MPI_INT, vet_menor, sendcounts, displacement,
              MPI_INT, 0, MPI_COMM_WORLD);
     
  if(myrank == 0)
  {
    // Imprime os resultados de vet_menor[]
    for (j = 0; j < dim; j++) 
    {
      printf("%d ", vet_menor[j]);
    }
    printf("\n");
  }

  // Libera memoria
  free(MAT);
  free(vet_menor);
  if(myrank == 0) free(somatorio);
  free(sendcounts);
  free(displacement);
  return !(MPI_Finalize() == MPI_SUCCESS);
}
