#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  double wtime;

  int *vetor, maior = -1, localmaior = -1, tam, num_threads;

  if (argc != 2) {
    printf("Wrong arguments. Please use main <amount_of_elements>\n");
    exit(0);
  }  // fim do if


  tam = atoi(argv[1]);

  printf("Amount of vetor=%d\n", tam);
  fflush(0);

  vetor = (int *)malloc(tam * sizeof(int));  // Aloca o vetor da dimensão lida

#pragma omp parallel private(localmaior) shared(maior)
  {
    localmaior = -1;
    num_threads = omp_get_num_threads();
    // iniciando vetor e fixando o maior valor para validacao
    // determina o nr da thread
    int my_id = omp_get_thread_num();

    wtime = omp_get_wtime();
    //----------------------------------
    // Calculo dos limites inferior e superior da thread
    int final = 0, inicio = my_id * (tam / num_threads),
        last_id = num_threads - 1;
    
    if (my_id != last_id)
      final = (my_id + 1) * (tam / num_threads);
    else
      final = tam;

    printf("Thread %d inicio = %d fim = %d\n", my_id, inicio, final - 1);
    //Preenchimento do setor do vetor com 1
    for (int i = inicio; i < final; i++) {
      vetor[i] = 1;
    }
    if (tam / 2 >= inicio && tam / 2 < final) vetor[tam / 2] = tam;
    //Final de atribuição de valores no vetor
    
    #pragma omp barrier

    for (int i = inicio; i < final; i++) {
      if (vetor[i] > localmaior) localmaior = vetor[i];
    }

    #pragma omp critical
    {
      if (localmaior > maior) 
        maior = localmaior;
      
    }
  }
  /*
   *************************************************************************************
    Não modifique este trecho de código
   */
  wtime = omp_get_wtime() - wtime;

  printf(
      "OMP NO FOR: Tam=%d, Num_Threads=%d, maior=%d, Elapsed wall clock "
      "time "
      "= %f  \n",
      tam, num_threads, maior, wtime);  // Imprime o vetor ordenado
  free(vetor);                          // Desaloca o vetor lido
  return 0;
}  // fim da main
