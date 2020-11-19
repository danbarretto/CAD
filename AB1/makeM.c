#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[]) {
  int n;
  n = atoi(argv[1]);
  srand(time(NULL));
  printf("%d\n%d\n", n, rand()%n);
  for(int i=0; i<n; i++){
      for(int j=0; j<n; j++){
        printf("%d ", rand()%1000);
      }
      printf("\n");
  }

  return 0;
}
