#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const* argv[]) {
  int n;
  scanf("%d\n", &n);
  fflush(stdin);
  char* frase = (char*)malloc(sizeof(char) * (n + 1));

  fgets(frase, n + 1, stdin);

  int maior = 0;
  #pragma omp parallel shared(maior)
  {
    #pragma omp single
    {
      char* saveptr = frase;
      char* subtoken = strtok(saveptr, " ");
      
      while (subtoken != NULL) {
        #pragma omp task firstprivate(subtoken)
        {
          int tam = strlen(subtoken);
          #pragma omp critical(atualizamaior)
          {
            if (maior < tam) maior = tam;
          }
        }
        subtoken = strtok(NULL, " ");
      }
    }
  }
  printf("%d\n", maior);

  free(frase);
  return 0;
}
