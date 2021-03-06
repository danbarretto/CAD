#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define INF 0x7fffffff

int **cria_matriz(int n, int m) {
  int **mat = (int **)malloc(sizeof(int *) * n);
  if (mat == NULL) return NULL;
  for (int i = 0; i < n; i++) {
    mat[i] = (int *)malloc(sizeof(int) * m);
    if (mat == NULL) {
      for (i = i - 1; i >= 0; i--) free(mat[i]);
      free(mat);
      return NULL;
    }
  }
  return mat;
}

int main(int argc, char const *argv[]) {
  int n, k;
  scanf("%d", &n);
  scanf("%d", &k);
  int **mat = cria_matriz(n, n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) scanf("%d", &mat[i][j]);
  }
  int i=0,min=INF;
  int* results = (int *)malloc(sizeof(int)*n);
  omp_set_nested(1);
  
  #pragma omp parallel for private(i,min) shared(results) 
  for (i = 0; i < n; i++) {
    int limiteInf = mat[i][k];
    int j;
    int localmin = INF;
    min = INF;
    #pragma omp parallel reduction(min: min) private(localmin)
    {
      localmin = INF;
      #pragma omp for private(j)
      for (j = 0; j < n; j++) {
        if (mat[i][j] < localmin && mat[i][j] > limiteInf)
          localmin = mat[i][j];
      }
      min = localmin;
    }  
    if(min == INF) min = -1;
    results[i] = min;
  }


  for (i = 0; i < n; i++){
    printf("%d ",results[i]);
    free(mat[i]);
  } 
  free(mat);
  free(results);
  printf("\n");

  return 0;
}