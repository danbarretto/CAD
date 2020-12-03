/*
Faz a soma dos elementos de dois vetores em uma stream e em outra stream faz a multiplicacao de um vetor
por um valor escalar

Exemplifica o uso de diferentes streams (1 e 2) para computacoes 
distintas. Usa cudaMallocHost para alocar memoria nao paginavel 
no host e faz copia assincrona com cudaMemcpyAsync. Usa tambem 
o cudaStreamSynchronize para aguardar toda a stream terminar.

O algoritmo calcula na stream 01 a soma de dois vetores e na stream 2
ele faz a multiplicacao de um vetor por um valor escalar.
O algoritmo divide "tam" elementos por "streams_nr" e encontra "threadsPerGrid" e "blocksPerGrid"
Os vetores no device tem o tamanho de threadsPerGrid.
Caso o nr de streams provoque uma divisao com resto, a ultima grade da stream deve acertar o resto.
Funcionou para teste com stream 03 e tam 16 ou 17 e TPB 2 ou 3

Caso o nr de threads por bloco provoque uma divisao com resto, o algoritmo deve funcionar 
com a fitragem do excesso implementada. Funcionou com alguns testes basicos.

Para compilar: nvcc 05-streams_MIMD.cu -o 05-streams_MIMD
Para executar: ./05-streams_MIMD

OBS: os valores de tamanho do vetor e o conteudo do vetor 
     estao fixos no codigo
*/


#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <time.h>

//Kernel que faz a soma de vetores
__global__ void soma(int *vetorA, int *vetorB,int *vetorC,int tam, int iter)
{ 
    int idx = blockDim.x * blockIdx.x + threadIdx.x; 
    if (idx < tam) 
    {
        vetorC[idx]=vetorA[idx]+vetorB[idx];
		printf("Soma: Iter=%d, Blc=%d, Thread=%d, Tam=%d, VetC[%d]=%d \n", iter, blockIdx.x, threadIdx.x, tam, idx, vetorC[idx]);
    }
}

//Kernel que faz a multiplicação de um escalar por um vetor
__global__ void mult_escalar(int *vetorA, int escalar,int tam, int iter)
{ 
    int idx = blockDim.x * blockIdx.x + threadIdx.x; 
    if (idx < tam) 
    {
        vetorA[idx]=escalar*vetorA[idx];
		printf("Mult: Iter=%d, Blc=%d, Thread=%d, Tam=%d, vet[%d]=%d \n", iter, blockIdx.x, threadIdx.x, tam, idx, vetorA[idx]);
    }
}

int main(int argc,char **argv)
{
    //Declara as variáveis de índice
    int i,threadsPerBlock; 

    //Inicializa a seed para geração de números pseudo aleatórios
    srand(time(NULL));

    //Declara os vetores no host
    int *vetorA,*vetorB,*vetorC,*vetorD;
    
    int escalar=10; //rand()%10+1;
    
    //Declara os vetores para uso na primeira stream
    int *vetorA_d1,*vetorB_d1,*vetorC_d1;

    //Declara o vetor para uso na segunda stream
    int *vetorD_d2;

    //Declaração das variáveis do tipo cudaStream_t
    cudaStream_t stream1,stream2;

    //Criação das streams
    cudaStreamCreate(&stream1);
    cudaStreamCreate(&stream2);

    //Define o tamanho do vetor
    int tam = 17;     //   16; // 2048;
    
    //Define a quantidade de threads por bloco
    threadsPerBlock = 3;  //2; //256; 

	//Define quantas streams usar
	int streams_nr = 3;
	
	//Define o nr de threads por grade (uma grade por vez na stream)
    int threadsPerGrid=tam/streams_nr;  // threadsPerGrid=8 (16/2) //  =1024 (2048/2)
	
    //Define a quantidade de blocos por grade
    int blocksPerGrid=(threadsPerGrid+threadsPerBlock-1)/threadsPerBlock;  // blockPerStream=4 (8/2) //  =4 (1024/256)
	// (8+1)/2 ==> 4 (4,5)
	// (1024+255)/256 ==> 4  (4,9960)
	//
   
    //Aloca memoria nao paginavel para os vetores no host
    cudaMallocHost((void**)&vetorA,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorB,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorC,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorD,tam*(sizeof(int))); 

    //Aloca os vetores no device para a stream 1
    cudaMalloc((void**)&vetorA_d1,threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorB_d1,threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorC_d1,threadsPerGrid*(sizeof(int))); 

    //Aloca os vetores no device para a stream 2
    cudaMalloc((void**)&vetorD_d2,threadsPerGrid*(sizeof(int))); 
  
    //Preenche os vetores no host
    for(i=0;i<tam;i++)
	{
        vetorA[i]=i;
        vetorB[i]=0; //i;
        vetorD[i]=i; //10;
    }
    
	printf("Host: tam=%d, streams_nr=%d, TPG=%d, BPG=%d, TPB=%d \n", tam, streams_nr, threadsPerGrid, blocksPerGrid, threadsPerBlock);

    for(i=0;i<tam;i+=threadsPerGrid)
	{
		// caso tenha mais streams que o necessario, precisa acertar o tamanho do bloco na ultima stream
		// essa ultima stream pega apenas o restante para processar; nao o vlr cheio de threadsPerGrid
		if((tam-i)< threadsPerGrid)
			threadsPerGrid = tam - i;


        //copia um bloco de tamanho threadsPerGrid do vetor A do host para o device (stream1)
        cudaMemcpyAsync(vetorA_d1,vetorA+i,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream1);
        //copia um bloco de tamanho threadsPerGrid do vetor B do host para o device (stream1)
        cudaMemcpyAsync(vetorB_d1,vetorB+i,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream1);

        //Invoca o kernel soma passando a stream 1 como argumento
        soma <<<blocksPerGrid,threadsPerBlock,0,stream1>>> (vetorA_d1,vetorB_d1,vetorC_d1,threadsPerGrid,i);

        //Copia um bloco de tamanho threadsPerGrid do resultado da stream 1 de volta para o host 
        cudaMemcpyAsync(vetorC+i,vetorC_d1,threadsPerGrid*(sizeof(int)),cudaMemcpyDeviceToHost,stream1);


        //copia um bloco de tamanho threadsPerGrid do vetor D do host para o device (stream2)
        cudaMemcpyAsync(vetorD_d2,vetorD+i,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream2);
		
        //Invoca o kernel mult_escalar passando a stream 2 como argumento
        mult_escalar <<<blocksPerGrid,threadsPerBlock,0,stream2>>> (vetorD_d2,escalar,threadsPerGrid,i);
		
        //Copia um bloco de tamanho threadsPerGrid do resultado da stream 2 de volta para o host
        cudaMemcpyAsync(vetorD+i,vetorD_d2,threadsPerGrid*(sizeof(int)),cudaMemcpyDeviceToHost,stream2);

    }
    //Sincroniza as streams
    cudaStreamSynchronize(stream1);
    cudaStreamSynchronize(stream2);

    printf("Soma dos vetores:\n");
    //Imprime o resultado da soma de vetores no host
    for(i=0;i<tam;i++)
	{
       printf("%d ",vetorC[i]);
    }
    printf("\n");
    printf("Multiplicação pelo escalar:\n");

    //Imprime o resultado da multiplicação pelo escalar no host
    for(i=0;i<tam;i++)
	{
        printf("%d ",vetorD[i]);
    }
    printf("\n");
    
    //Desaloca os vetores no host
    cudaFreeHost(vetorA);
    cudaFreeHost(vetorB);
    cudaFreeHost(vetorC); 
    cudaFreeHost(vetorD); 
    
    //Desaloca os vetores da stream 1
    cudaFree(vetorA_d1);
    cudaFree(vetorB_d1);
    cudaFree(vetorC_d1);

    //Desaloca o vetor da stream 2
    cudaFree(vetorD_d2);
    
    //Destroi as streams
    cudaStreamDestroy(stream1);
    cudaStreamDestroy(stream2);
}