#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  int n, k;
  scanf("%d", &n);
  scanf("%d", &k);
  int *vet = (int *)malloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) scanf("%d", &vet[i]);

  int limiteInf = vet[k];
  int min = -1, i;
  int localmin = -1;
  omp_lock_t mylock;
  omp_init_lock(&mylock);
  #pragma omp parallel shared(min) private(localmin)
  {
    localmin = -1;
    #pragma omp for private(i)
    for (i = 0; i < n; i++) {
      if ((localmin == -1 || vet[i] < localmin) && vet[i] > limiteInf)
        localmin = vet[i];
    }
    omp_set_lock(&mylock);
    if ((min == -1 || localmin < min) && localmin != -1) min = localmin;
    omp_unset_lock(&mylock);

  }  // fim da regiao paralela
  printf("%d\n", min);
  free(vet);
  return 0;
}
