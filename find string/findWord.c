/*
    Daniel Sá Barretto Prado Garcia 10374344
    Felipe Guilermmo Santuche Moleiro 10724010
    Laura Genari Alves de Jesus 10801180 
    Tiago Marino Silva 10734748
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
    //Tamanho da matriz, qtde de buscas, tamanho de uma palavra de busca
    int L, C, W, TamPalavra;

    //Matriz de caracteres, Vetor de palavras para a busca, var auxiliar
    char *Matriz, *VetPalavras;
    
    //============= MPI ==================//
    int npes, myrank, src, dest, msgtag;
    int token;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if(myrank == 0) 
    {
        FILE *inputfile;     // handler para o arquivo de entrada
        char inputfilename[256];
        
        if (argc != 2){
            printf("Nr errado de argumentos. Execute passando <arq_entrada> como argumento.\n");
            return(0);
        }

        strcpy(inputfilename,argv[1]);

        inputfile = fopen(inputfilename, "r");
        if(!inputfile){
            printf("Mestre: Erro ao abrir arquivo de entrada %s. Saindo. \n", argv[1]);
            fflush(0);
            return(0);
        }

        //Leitura dos dados de entrada:
        // carrega nr de linhas e colunas da matriz de caracteres em Ordem
        fscanf(inputfile, "%d %d\n", &L, &C);

        // aloca um vetor de tamanho L de ponteiros para as linhas
        Matriz = (char *) malloc (L * (C+1) * sizeof (char));

        //Leitura da matriz de caracteres em Matriz.
        //Leitura ocorre por linha, ate encontrar o final da linha ou espaco em branco/tab/...
        for(int i = 0; i < L; i++)
        {
            fscanf(inputfile, "%s", &Matriz[i*(C+1)]);
        }

        fscanf(inputfile, "%d\n", &W);

        // aloca um vetor de tamanho W de ponteiros para as linhas
        TamPalavra = (C+1);
        VetPalavras = (char*) malloc (W * TamPalavra * sizeof (char));

        // Leitura das palavras a serem procuradas em VetPalavras
        for(int i = 0; i < W; i++)
        {
            fscanf(inputfile, "%s", &VetPalavras[i*TamPalavra]);
        }
        // fecha o arquivo de entrada
        fclose(inputfile);
    }
    
    // Broadcast L, C e W
    MPI_Bcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&C, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&W, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(myrank != 0)
    {
        TamPalavra = C + 1;
        Matriz = (char *) malloc (L * (C+1) * sizeof (char));
    }

    // Broadcast Matriz
    MPI_Bcast(Matriz, L * TamPalavra, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    /* Divide palavras de W para cada processo */
    int num_palavras = W / npes;
    int num_palavras_resto = num_palavras + (W % npes);
    

    int *sendcounts = (int*) malloc(sizeof(int) * npes);
    int *displacement = (int*) malloc(sizeof(int) * npes);

    // All processes have N * N / npes integers to compute
    // Calculate displacement
    for (int i = 0; i < npes; i++) {
        sendcounts[i] = (num_palavras * TamPalavra);
        displacement[i] = (num_palavras * TamPalavra) * i;
    }
    // Last process has the missing part
    sendcounts[npes - 1] = num_palavras_resto * TamPalavra;

    int num_linhas = myrank == npes - 1 ? num_palavras_resto : num_palavras;
    
    // Aloca memoria para receber as palavras
    if(myrank != 0){
        VetPalavras = (char*) malloc (num_linhas * TamPalavra * sizeof (char));
    }
    
    // Faz o scatter das palavras
    if (myrank == 0 || num_linhas != 0) 
    {
        MPI_Scatterv(VetPalavras, sendcounts, displacement, MPI_CHAR, VetPalavras, 
        num_linhas*TamPalavra, MPI_CHAR, 0, MPI_COMM_WORLD);
    }


    // Alocacao de memoria para o resultado
    int *colunas, *linhas;
    if(myrank ==0){
        colunas = (int*) malloc(W * sizeof(int));
        linhas = (int*) malloc(W * sizeof(int));
    }else{
        colunas = (int*) malloc(num_linhas * sizeof(int));
        linhas = (int*) malloc(num_linhas * sizeof(int));
    }

    int j = 0, i = 0;
    int linha = -1;
    int coluna = -1;
    
    // Calcula posicao da substring
    #pragma omp parallel for private(i,j,linha,coluna) shared(colunas, linhas)
    for(i=0; i<num_linhas; i++){
        char* substrPointer = NULL;
        linha = -1;
        coluna = -1;
        for(j=0;j<L;j++)
        {
            // strstr procura por substring e retorna o endereco do inicio da substring
            // ou nulo se nao encontrar
            substrPointer = strstr(&Matriz[j * TamPalavra], &VetPalavras[i * TamPalavra]);
            // Se encontrou calcula a posicao da coluna e linha e para a busca para essa palavra
            if(substrPointer != NULL)
            {
                coluna = (int) (substrPointer - &Matriz[j * TamPalavra]);
                linha = j;
                break;
            }
        }
        // Salva resultado para a palavra
        linhas[i] = linha;
        colunas[i] = coluna;
    }

    // Calculo do sendcount e displacement para o gatherv
    for (int i = 0; i < npes; i++) {
        sendcounts[i] = num_palavras;
        displacement[i] = num_palavras * i;
    }
    // Last process has the missing part
    sendcounts[npes - 1] = num_palavras_resto;
    
    MPI_Gatherv(colunas, num_linhas, MPI_INT, colunas, sendcounts, displacement,
              MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(linhas, num_linhas, MPI_INT, linhas, sendcounts, displacement,
              MPI_INT, 0, MPI_COMM_WORLD);
    
    // Printa resultados
    if(myrank == 0)
    {
        for(int i = 0; i < W; i++){
            printf("%s,%d,%d\n", &VetPalavras[i * TamPalavra], linhas[i], colunas[i]);
            fflush(0);
        }
    }

    // Limpa Memoria             
    free(Matriz);
    free(VetPalavras);
    free(sendcounts);
    free(displacement);
    free(linhas);
    free(colunas);
    return !(MPI_Finalize() == MPI_SUCCESS);
}