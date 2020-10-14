#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// BEM VINDO !!!!

double** cria_matriz(int n, int m) {
  double** mat = (double**)malloc(sizeof(double*) * n);
  if (mat == NULL) return NULL;
  for (int i = 0; i < n; i++) {
    mat[i] = (double*)malloc(sizeof(double) * m);
    if (mat == NULL) {
      for (i = i - 1; i >= 0; i--) free(mat[i]);
      free(mat);
      return NULL;
    }
  }
  return mat;
}

/*double moda(double* col, int tam_col) {
  int v[tam_col];
  for(int i =0; i<tam_col ;i++){
    v[col[i]%tam_col]++;
  }
}*/

int cmp(const void* a, const void* b) {
  if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;
}

double mediana(double* col, int tam_col) {
  if (tam_col % 2 != 0) {
    double res = col[(int)(tam_col / 2)];
    return res;
  } else {
    double res = (col[(tam_col / 2) - 1] + col[tam_col / 2]) / 2;
    return res;
  }
}

double moda(double* col, int tam_col) {
  double res;
  for (int i = 0; i < tam_col; i++) {
    col[i] = col[i];
  }
  return res;
}

double media_aritmetica(double* col, int tam_col) {
  double media = 0;
  for (int i = 0; i < tam_col; i++) {
    media += col[i];
  }
  media /= tam_col;
  return media;
}

double media_harmonica(double* col, int tam_col) {
  double media = 0;
  for (int i = 0; i < tam_col; i++) {
    media += 1 / col[i];
  }
  return tam_col / media;
}

double variancia(double* col, int tam_col, double media) {
  double var = 0;
  for (int i = 0; i < tam_col; i++) {
    var += (col[i] - media) * (col[i] - media);
  }
  var /= (tam_col - 1);  // trixtão D:
  return var;
}

double desvio_padrao(double var) { return sqrt(var); }

double coef_variacao(double desv, double media) { return desv / media; }

int main(int argc, char const* argv[]) {
  int n, m;
  omp_set_nested(1);

  scanf("%d %d", &n, &m);
  // double mediaA[m], mediaH[m], median[m], moda[m], var[m], desv[m], coef[m];

  double* mediaA = malloc(sizeof(double) * m);
  double* mediaH = malloc(sizeof(double) * m);

  double wtime;
  double** mat = cria_matriz(m, n);
  double** cpy = cria_matriz(m, n);
  if (mat == NULL || cpy == NULL) {
    printf("SEM MEMORIA PARA ALOCAR MATRIZ!!\n");
    return 0;
  }

  // Leitura da matriz transposta para facilitar o uso de colunas como
  // parâmetros

  wtime = omp_get_wtime();
  //omp_set_num_threads(8);
  int i = 0;
    #pragma omp parallel shared(mat, cpy, mediaA, mediaH)
  {
    #pragma omp single
    {
      #pragma omp task  // media aritimetica
      {
        printf("Thread %d das %d", omp_get_thread_num(),omp_get_num_threads());
        #pragma omp parallel for private(i)
          printf("num_threads: %d\n", omp_get_num_threads());
          for (int i = 0; i < m; i++) mediaA[i] = media_aritmetica(mat[i], n);
      }
      #pragma omp task  // media harmonica
      {
        #pragma omp parallel for private(i)
          for (int i = 0; i < m; i++) mediaH[i] = media_harmonica(mat[i], n);
      }
      #pragma omp task depend(out : cpy[i])  // qsort
      {
        #pragma omp parallel for private(i)
          for (int i = 0; i < m; i++) qsort(cpy[i], n, sizeof(double), cmp);
      }
    }
  }

  wtime = omp_get_wtime() - wtime;

  printf("Tempo de execução paralela %lf\n", wtime);

  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++) cpy[i][j] = mat[i][j];

  // SEQUENCIAL-------------------------
  clock_t start = clock();
  for (int i = 0; i < m; i++) {
    double media;
    media = media_aritmetica(mat[i], n);
    mediaA[i] = media_harmonica(mat[i], n);
    mediaA[i] = variancia(mat[i], n, media);
    mediaA[i] = desvio_padrao(mediaA[i]);
    qsort(cpy[i], n, sizeof(double), cmp);
    mediaA[i] = mediana(cpy[i], n);
    // moda[i] = moda
  }
  clock_t end = clock();
  //---------------------------------------

  printf("Tempo de execução sequencial: %lf\n",
         (double)(end - start) / (double)CLOCKS_PER_SEC);

  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++) cpy[i][j] = mat[i][j];

  wtime = omp_get_wtime();
  omp_set_num_threads(8);

  for (int i = 0; i < m; i++) free(mat[i]);
  free(mat);

  for (int i = 0; i < m; i++) free(cpy[i]);
  free(cpy);

  free(mediaA);
  free(mediaH);

  return 0;
}

// task 1 - calcular medias
// parelelizar aritmetica e harmonica

// task 2 - ordenar vetor

// task 3 - mediana e moda --> depende da 2
// paralelizar

// task 4 - var - desv - coef --> depede da 1