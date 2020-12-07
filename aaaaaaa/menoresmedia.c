#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv)
{
    //Declara a matriz MAT e o vetor vet_menor
    int *MAT, *vet_menor;

    //Declara as variáveis de índice
    int i,j,dim;
	
    //Declara a media para MAT
    double media_MAT;

    FILE *inputfile;     // handler para o arquivo de entrada
    char *inputfilename;  // nome do arquivo de entrada

   
    if (argc < 2)
    {
		printf("Nr errado de argumentos. Execute passando <arq_entrada> como argumento. \n");
		exit(-1);
    }

    inputfilename = (char*) malloc (256*sizeof(char));
    strcpy(inputfilename,argv[1]);
/*    
    // para depuracao apenas
	printf("inputfilename=%s\n", inputfilename);
	fflush(0);
*/

    if ((inputfile=fopen(inputfilename,"r")) == 0)
    {
		printf("Mestre: Erro ao abrir arquivo de entrada %s. Saindo. \n", inputfilename);
		exit(-1);
    }
    
    fscanf(inputfile, "%d\n", &dim); //Lê a dimensão de MAT

/*    
    // para depuracao apenas
	printf("dim=%d\n", dim);
	fflush(0);
*/
    
    //Aloca a matriz
    MAT = (int *)malloc(dim * dim * sizeof(int));

    //Aloca um vetor para armazenar a quantide de nrs menores que a media por coluna
    vet_menor = (int *)malloc(dim * sizeof(int));

     //Lê a matriz MAT
	 for(i=0;i<dim;i++)
	{
        for(j=0;j<dim;j++)
		{
            fscanf(inputfile, "%d ", &(MAT[i*dim+j]));
        }
    }

	// fecha o arquivo de entrada
	fclose(inputfile);

/*    //para depuracao apenas
	for(i=0;i<dim;i++)
	{
        for(j=0;j<dim;j++)
		{
            printf("%d ", MAT[i*dim+j]);
        }
        printf("\n");
        fflush(0);
    }
*/
	// incializa com zero as posicoes de vet_menor
	for (j = 0; j< dim; j++)
	{
		vet_menor[j] = 0;
	}

/*	// para depuracao apenas
    printf("Atribuiu zero a vet_menor[] \n");
    fflush(0);
*/    
    
	media_MAT = .0; 
    //Calcula a media de MAT
    for(i=0;i<dim;i++)
	{
        for(j=0;j<dim;j++)
		{
            media_MAT += (double)MAT[i*dim+j];
        }
    }
	media_MAT = (double) media_MAT / (double) (dim*dim);
	
/*	// para depuracao apenas
    printf("media = %.1lf \n", media_MAT);
    fflush(0);
*/   
    
    //encontra os valores de MAT[] < media_MAT
    for(j=0; j<dim; j++)
	{
        for(i=0; i<dim; i++)
		{
			if (MAT[i*dim+j] < media_MAT)
			{
				vet_menor[j]++;
			}
        }
    }
	
    //Imprime os resultados de vet_menor[]
	for (j = 0; j < dim; j++)
	{
		printf("%d ", vet_menor[j]); 
	}
	printf("\n");
	
    //Libera MAT[] e vet_menor[]
    free(MAT);
    free(vet_menor);

	
	return(0);
}
