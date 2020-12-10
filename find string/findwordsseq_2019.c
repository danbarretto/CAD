/*
Dada uma matriz de dimensoes LxC, com digitos decimais (0-9),
procurar por palavras nas linhas da matriz. As palavras a serem 
procuradas tem tamanhos variaveis nao maiores que C caracteres validos 
(sem contar o '\0') e assume-se que elas aparecem apenas uma vez na matriz,
i.e., nao se repetem.
No final, a aplicacao mostra como resultado, para cada palavra, 
a sua localizacao inicial na matriz em termos de linha e coluna, 
(ambas iniciando em zero). Se a palavra nao for encontrada na 
matriz, os endereços de linha e coluna no arquivo de saida devem ser ambos -1.

O arquivo de entrada contem em cada linha:
- valores de L e C para a matriz de caracteres;
- matriz de caracteres com digitos decimais (0-9). Cada linha da matriz 
  esta em uma linha do arquivo de entrada.
- quantidade de W palavras a serem buscadas na matriz.
- palavras a serem buscadas na matriz, sendo, novamente, uma palavra por linha.

O arquivo de saida contem uma linha para cada palavra a ser procurada na matriz, 
com os seguintes resultados: <palavra>,<linha>,<coluna>  (sem espaços entre eles e 
com quebra de linha no final de cada linha) 

Exemplo de arquivo de entrada:
10           << ordem da matriz
0000012345   << digitos da matriz
9876543111
5122222222
3333753193
4445494444
0695555555
6666826666
7777777777
2929292988
1818181818	<< digitos da matriz
12			<< nr de palavras a buscar
12345       << palavra a buscar
9876543		<< palavra a buscar
512
75319		<< palavra a buscar
549
69
826
0101		<< palavra a buscar. Esta não existe.
29292929
1818181818	<< palavra a buscar
3416		<< palavra a buscar
4444

Exemplo de arquivo de saida:
12345,0,5       << palavra a buscar
9876543,1,0		<< palavra a buscar
512,2,0
75319,3,4		<< palavra a buscar
549,4,3
69,5,1
826,6,4
0101,-1,-1		<< palavra a buscar. Esta não existe.
29292929,8,0
1818181818,9,0	<< palavra a buscar
3416,-1,-1		<< palavra a buscar
4444,4,6		<< palavra a buscar

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Based on Author: Paulo S L de Souza

int encontra(char *Frase, char *Palavra, int C, int TamPalavra)
{
      int DeslocaPalavra, DeslocaMatriz, ColIni;

      DeslocaPalavra = 0;  // percorre caracteres da palavra de busca (Palavra)
      DeslocaMatriz = 0;   // percorre caracteres da linha da matriz (Frase)
      ColIni = -1;         // guarda a posicao inicial da Palavra na Frase (coluna onde inicia)
   
      
      // enquanto ha deslocamentos na linha da matriz a fazer para buscar a palavra de TamPalavra 
      // E
      // enquanto a palavra a ser buscada ainda nao foi percorrida por completo
//      printf("Frase=%s e Palavra=%s\n", Frase, Palavra);
//      fflush(0);
      
      while( (((C+1) - DeslocaMatriz) >= (TamPalavra - DeslocaPalavra))  &&  (DeslocaPalavra < TamPalavra) )
      {
            if(Frase[DeslocaMatriz] == Palavra[DeslocaPalavra]) // se caracteres sao iguais
            {
                if(DeslocaPalavra == 0) // eh o primeiro a ser igual?
                {
                    ColIni = DeslocaMatriz; // marca o inicio da palavra na Matriz. Vai que serah igual!
                }
                DeslocaPalavra++;  // ainda é igual, entao desloca caracter na palavra de busca
            }
            else
            {
                DeslocaPalavra=0; // volta ao comeco da palavra de busca, pois nao eh igual
                ColIni=-1;
            }
            DeslocaMatriz++;  // desloca o caracter na Matriz
/*         // para depuracao apenas 
           printf("ColIni=%d, DeslocaPalavra=%d, DeslocaMatriz=%d, TamPalavra=%d, C+1=%d, %c==%c?.\n ", ColIni, DeslocaPalavra, DeslocaMatriz, TamPalavra, C+1, Frase[DeslocaMatriz], Palavra[DeslocaPalavra]);
            fflush(0);
*/
      } // fim do while

/*  
    // para depuracao apenas
    printf("ColIni=%d, DeslocaPalavra=%d, DeslocaMatriz=%d, TamPalavra=%d.\n ", ColIni, DeslocaPalavra, DeslocaMatriz, TamPalavra);
	fflush(0);
*/

      if(DeslocaPalavra != TamPalavra)
      {
            ColIni = -1;
      }

// 	ColIni tem a posicao inicial da palavra na frase ou -1 para indicar que nao encontrou
//	printf("Fim da funcao Encontra(): String %s na coluna %d.\n", Palavra, ColIni);
 
      return(ColIni);
      
} // fim encontra



int main(int argc, char **argv)
{

    //Tamanho da matriz, qtde de buscas, tamanho de uma palavra de busca
    int L, C, W, TamPalavra;

    //Matriz de caracteres, Vetor de palavras para a busca, var auxiliar
    char **Matriz, **VetPalavras, *Palavra, *Frase;

    //Variaveis auxiliares
    int i, linha, LinhaMatriz, Coluna;
    
    FILE *inputfile;     // handler para o arquivo de entrada
	//char *inputfilename;  // nome do arquivo de entrada
    char inputfilename[256];
    
	if (argc != 2)
	{
		printf("Nr errado de argumentos. Execute passando <arq_entrada> como argumento.\n");
		return(0);
	}


	//inputfilename = (char*)calloc(28, sizeof(char));
	strcpy(inputfilename,argv[1]);

/*
      // para depuracao
        printf("Mestre: inputfilename=%s.\n", inputfilename);
		fflush(0);
*/

    inputfile = fopen(inputfilename, "r");
	if(!inputfile)
	{
		printf("Mestre: Erro ao abrir arquivo de entrada %s. Saindo. \n", argv[1]);
		fflush(0);
		return(0);
	}

/*
      // para depuracao
        printf("Mestre: inputfilename=%s\n", inputfilename);
		fflush(0);
*/
    
    //Leitura dos dados de entrada:
    // carrega nr de linhas e colunas da matriz de caracteres em Ordem
    fscanf(inputfile, "%d %d\n", &L, &C);
/*    
    // para depuracao
    printf("Mestre: L=%d, C=%d. \n", L, C);
    fflush(0);
*/

    // aloca um vetor de tamanho L de ponteiros para as linhas
    Matriz = (char **) malloc (L * sizeof (char*));

    // aloca cada uma das linhas (vetores de COL inteiros)
    for (i=0; i < L; i++)
        Matriz[i] = (char*) malloc ((C+1) * sizeof (char)) ;
    
 
    //Leitura da matriz de caracteres em Matriz.
    //Leitura ocorre por linha, ate encontrar o final da linha ou espaco em branco/tab/...
    for(linha = 0; linha < L; linha++)
    {
            fscanf(inputfile, "%s", Matriz[linha]);
/*            // para depuracao
            printf("Matriz[%d]=%s. strlen=%ld.\n", linha, Matriz[linha], strlen(Matriz[linha])); 
            fflush(0);
*/        
    }

 /*   // para depuracao
    printf("Mestre: passou for(linha...) \n");
    fflush(0);
*/
/*    
    //imprimindo Matriz apenas para conferencia
    //imprime caracter por caracter porque nao tem o '\0'
    for(linha = 0; linha < L; linha++){
        printf("%s\n", Matriz[linha]);
        fflush(0);
    }
*/
    
    //Leitura da quantidade de palavras a buscar na matriz
    // carrega qtde de palavras a serem buscadas em QtdePalavras
    fscanf(inputfile, "%d\n", &W);

/*
    printf("QtdePalavras(W)=%d. \n", W);
    fflush(0);
*/

    // aloca um vetor de tamanho W de ponteiros para as linhas
    VetPalavras = (char**) malloc (W * sizeof (char*));

    // aloca cada uma das strings para VetPalavras (vetores de C+1 caracteres)
    for (i=0; i < W; i++)
        VetPalavras[i] = (char *) malloc ((C+1) * sizeof (char)) ;

    
    // Leitura das palavras a serem procuradas em VetPalavras
    for(i = 0; i < W; i++)
    {
        fscanf(inputfile, "%s", VetPalavras[i]);
    }

/*    //imprimindo VetPalavras apenas para depuracao
    for(i = 0; i < W; i++)
    {
        printf("VetPalavras[%d]=%s. strlen()=%ld\n ", i, VetPalavras[i], strlen(VetPalavras[i]));
    }
*/

    // fecha o arquivo de entrada
    fclose(inputfile);

    
    //aloca variavel auxiliar Frase, para receber string de uma linha de Matriz
    Frase=(char*)malloc((C+1)*sizeof(char));

    //aloca variavel auxiliar Palavra, a ser procurada na Matriz
    Palavra=(char*)malloc((C+1)*sizeof(char));

    //  *********************************************************************************
    //  *********************************************************************************
    for(linha = 0; linha < W; linha++)
    {
        strcpy(Palavra, VetPalavras[linha]);
        TamPalavra = strlen(Palavra);
        
/*      // para depuracao apenas
	    printf("Palavra=%s, TamPalavra=%d.\n ", Palavra, TamPalavra);
        fflush(0);
*/
        for(LinhaMatriz = 0; LinhaMatriz < L; LinhaMatriz++)
        {
            strcpy(Frase, Matriz[LinhaMatriz]);

            /*
            // imprime o resultado do algoritmo
            printf("LinhaMatriz=%d, Frase=%s.\n ", LinhaMatriz, Frase);
            fflush(0);
*/
            Coluna = encontra(Frase, Palavra, C, TamPalavra);

            if(Coluna != -1)
            {
/*		        printf("Achou a string %s na linha %d e coluna %d.\n", Palavra, LinhaMatriz, Coluna);
		        fflush(0);
*/
                break;
            }
	
        } // fim for linhamatriz

        // se a coluna for -1 não encontrou a string. Coloca a linha como -1 tambem.
        if(Coluna == -1)
            LinhaMatriz = -1;
 
        // saida do algoritmo no padrao especificado.
        printf("%s,%d,%d\n", Palavra, LinhaMatriz, Coluna);
        fflush(0);

/*      // para depuracao
	    if(Coluna != -1)
        {
            printf("Achou a string %s na linha %d e coluna %d.\n", Palavra, LinhaMatriz, Coluna);
            fflush(0);
        }
        else
        {
            printf("Não achou a string %s na matriz.\n", Palavra);
            fflush(0);
        }
*/

    } // fim do for(linha) para as palavras a serem buscadas
    //  *********************************************************************************
    //  *********************************************************************************


    free(Frase);
    free(Palavra);

    for (i=0; i < L; i++)
        free(Matriz[i]);
    free(Matriz);
    
    for (i=0; i < W; i++)
        free(VetPalavras[i]);
    free(VetPalavras);

    free(inputfile);
        
    return(0);
    
}  // fim da main
