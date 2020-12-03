/*
Faz a soma dos elementos de dois vetores 

Exemplifica o uso de diferentes streams (1 e 2) com cudaMallocHost
para alocar memoria no host nao paginavel e copia assincrona 
com cudaMemcpyAsync. Usa tambem o cudaStreamSynchronize para
aguardar toda a stream terminar.

O algoritmo divide "tam" elementos por "streams_nr*2" e encontra "threadsPerGrid" e "blocksPerGrid".
Sao invocadas duas streams (1 e 2) a cada iteracao do loop for, por isso a multiplicacao por 2.
O vetor no device tem o tamanho de threadsPerGrid.
Caso o nr de streams provoque uma divisao com resto, a ultima grade da stream acerta o resto
Caso o nr de threads por bloco provoque uma divisao com resto, o algoritmo funciona fitra o excesso


Para compilar: nvcc 04-soma-vet-stream-2.cu -o 04-soma-vet-stream-2
Para executar: ./04-soma-vet-stream-2

OBS: os valores de tamanho do vetor e o conteudo do vetor 
     estao fixos no codigo
*/

#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

__global__ void soma(int *vetorA, int *vetorB, int *vetorC, int tam, int iter, int strm)
{ 
    int idx = blockDim.x * blockIdx.x + threadIdx.x; 
    if (idx < tam) 
    {
        vetorC[idx]=vetorA[idx]+vetorB[idx];
		printf("Iter=%d, Stream=%d, Blc=%d, Thread=%d, Tam=%d, VetC[%d]=%d \n", iter, strm, blockIdx.x, threadIdx.x, tam, idx, vetorC[idx]);

    }
}

int main(int argc,char **argv)
{
    int i,*vetorA,*vetorB,*vetorC,threadsPerBlock; 
    
    //Declara os vetores para uso na primeira stream
    int *vetorA_d1,*vetorB_d1,*vetorC_d1;

    //Declara os vetores para uso na segundo stream
    int *vetorA_d2,*vetorB_d2,*vetorC_d2;

    //Declaração da variável do tipo cudaStream_t
    cudaStream_t stream1,stream2;

    //Criação das streams
    cudaStreamCreate(&stream1);
    cudaStreamCreate(&stream2);

    //Define o tamanho do vetor, multiplo de 256
    int tam = 16;     //   16; // 2048;
    
    //Define a quantidade de threads por bloco
    threadsPerBlock = 2;  //2; //256; 

	//Define quantas streams usar
	int streams_nr = 2;
	
	//Define o nr de threads por grade (uma grade por vez na stream)
	// divide por 2 pq sao executadas duas grades por iteracao do for abaixo.
	// cada grade ira executar em uma stream diferente (stream 1 e stream 2)
    int threadsPerGrid=(tam/streams_nr)/2;  // threadsPerGrid=4 (16/2)/2 //  =512 (2048/2)/2
	// 17 / 2 / 2 = 4 (8,5 / 2 = 4,25)
	
    //Define a quantidade de blocos por grade
    int blocksPerGrid=(threadsPerGrid+threadsPerBlock-1)/threadsPerBlock;  // blockPerStream=4 (8/2) //  =4 (1024/256)
	// (4+1)/2 ==> 2 (2,5)
	// (512+255)/256 ==> 2  (2,9960)
	// (4+1)/2 ==> 2  (2,5)


    //Aloca os vetores no host usando pinned pages 
    cudaMallocHost((void**)&vetorA,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorB,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorC,tam*(sizeof(int))); 

    //Aloca os vetores no device para a stream 1
    cudaMalloc((void**)&vetorA_d1,threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorB_d1,threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorC_d1,threadsPerGrid*(sizeof(int))); 

    //Aloca os vetores no device para a stream 2
    cudaMalloc((void**)&vetorA_d2,threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorB_d2,threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorC_d2,threadsPerGrid*(sizeof(int))); 
    
    //Preenche os vetores no host
    for(i=0;i<tam;i++)
	{
        vetorA[i]= i;
        vetorB[i]= 0; //-i;
    }

	printf("Host: tam=%d, streams_nr=%d, TPG=%d, BPG=%d, TPB=%d \n", tam, streams_nr, threadsPerGrid, blocksPerGrid, threadsPerBlock);

    
    for(i=0;i<tam;i+=threadsPerGrid*2)   // i+=4*2   // i+=512*2   (*2 pq sao duas streams por iter)
	{
		// caso tenha mais streams que o necessario, precisa acertar o tamanho do bloco na ultima stream
		// essa ultima stream pega apenas o restante para processas; nao o vlr cheios de threadsPerGrid
		if((tam-i)< (threadsPerGrid*2))
			threadsPerGrid = tam - i;

        //copia um bloco de tamanho threadsPerGrid do vetor A do host para o device (stream1)
        cudaMemcpyAsync(vetorA_d1,vetorA+i,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream1);
 
		//copia um bloco de tamanho threadsPerGrid do vetor B do host para o device (stream1)
        cudaMemcpyAsync(vetorB_d1,vetorB+i,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream1);

        //Invoca o kernel soma passando a stream 1 como argumento
        soma <<<blocksPerGrid,threadsPerBlock,0,stream1>>> (vetorA_d1,vetorB_d1,vetorC_d1,threadsPerGrid,i,1);

        //Copia um bloco de tamanho bloco do resultado da stream 1 de volta para o host 
        cudaMemcpyAsync(vetorC+i,vetorC_d1,threadsPerGrid*(sizeof(int)),cudaMemcpyDeviceToHost,stream1);

		if(i+threadsPerGrid < tam)
		{ // se a segunda stream ainda tem computacao a fazer...
			//copia um bloco de tamanho bloco do vetor A do host para o device (stream2)
			cudaMemcpyAsync(vetorA_d2,vetorA+i+threadsPerGrid,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream2);
			//copia um bloco de tamanho bloco do vetor B do host para o device (stream2)
			cudaMemcpyAsync(vetorB_d2,vetorB+i+threadsPerGrid,threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream2);
			
			//Invoca o kernel soma passando a stream 2 como argumento
			soma <<<blocksPerGrid,threadsPerBlock,0,stream2>>> (vetorA_d2,vetorB_d2,vetorC_d2,threadsPerGrid,i,2);
			
			//Copia um bloco de tamanho bloco do resultado da stream 2 de volta para o host
			cudaMemcpyAsync(vetorC+i+threadsPerGrid,vetorC_d2,threadsPerGrid*(sizeof(int)),cudaMemcpyDeviceToHost,stream2);
		}
    }
    //Sincroniza as streams
    cudaStreamSynchronize(stream1);
    cudaStreamSynchronize(stream2);

    //Imprime o resultado no host
    for(i=0;i<tam;i++)
	{
       printf("%d ",vetorC[i]);
    }
    
    //Desaloca os vetores no host
    cudaFreeHost(vetorA);
    cudaFreeHost(vetorB);
    cudaFreeHost(vetorC); 
    
    //Desaloca os vetores da stream 1
    cudaFree(vetorA_d1);
    cudaFree(vetorB_d1);
    cudaFree(vetorC_d1);

    //Desaloca os vetores da stream 2
    cudaFree(vetorA_d2);
    cudaFree(vetorB_d2);
    cudaFree(vetorC_d2);
    
    //Destroi as streams
    cudaStreamDestroy(stream1);
    cudaStreamDestroy(stream2);
}
