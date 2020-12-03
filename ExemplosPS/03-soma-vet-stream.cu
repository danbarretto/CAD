/*
Faz a soma dos elementos de dois vetores 

Exemplifica o uso de diferentes streams com cudaMallocHost
para alocar memoria no host nao paginavel e copia assincrona 
com cudaMemcpyAsync. Usa tambem o cudaStreamSynchronize para
aguardar toda a stream terminar.
O algoritmo divide "tam" elementos por "streams_nr" e encontra "threadsPerGrid" e "blocksPerGrid"
O vetor no device tem o tamanho de threadsPerGrid.
Caso o nr de streams provoque uma divisao com resto, a ultima grade da stream deve acertar o resto.
Funcionou para teste com stream 03 e tam 16 ou 17 e TPB 2 ou 3

Caso o nr de threads por bloco provoque uma divisao com resto, o algoritmo deve funcionar 
com a fitragem do excesso implementada. Funcionou com alguns testes básicos.


Para compilar: nvcc 03-soma-vet-stream.cu -o 03-soma-vet-stream
Para executar: ./03-soma-vet-stream

OBS: os valores de tamanho do vetor e o conteudo do vetor 
     estao fixos no codigo
*/


#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

__global__ void soma(int *vetorA, int *vetorB,int *vetorC, int tam, int iter)
{ 
    int idx = blockDim.x * blockIdx.x + threadIdx.x; 
    if (idx < tam) 
    {
        vetorC[idx]=vetorA[idx]+vetorB[idx];
		printf("Iter=%d, Blc=%d, Thread=%d, Tam=%d, VetC[%d]=%d \n", iter, blockIdx.x, threadIdx.x, tam, idx, vetorC[idx]);
    }
}

int main(int argc,char **argv)
{
    int i,*vetorA,*vetorB,*vetorC,threadsPerBlock; 
    int *vetorA_d,*vetorB_d,*vetorC_d;

    //Declaração da variável do tipo cudaStream_t
    cudaStream_t stream;

    //Criação da stream
    cudaStreamCreate(&stream);

    //Define o tamanho do vetor, multiplo de 256
    int tam = 16;     //   16; // 2048;
    
    //Define a quantidade de threads por bloco
    threadsPerBlock = 2;  //2; //256; 

	//Define quantas streams usar
	int streams_nr = 2;
	
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

    //Aloca os vetores no device
    cudaMalloc((void**)&vetorA_d, threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorB_d, threadsPerGrid*(sizeof(int))); 
    cudaMalloc((void**)&vetorC_d, threadsPerGrid*(sizeof(int))); 
    
    //Preenche os vetores no host
    for(i=0;i<tam;i++)
	{
        vetorA[i]=i;
        vetorB[i]=0;
    }
    
	printf("Host: tam=%d, streams_nr=%d, TPG=%d, BPG=%d, TPB=%d \n", tam, streams_nr, threadsPerGrid, blocksPerGrid, threadsPerBlock);

	// a cada iteracao desloca o bloco em threadsPerGrid itens
	// equivale a deslocar streams_nr
    for(i=0; i<tam; i+=threadsPerGrid)    //i+=8  //i+1024
	{
		// caso tenha mais streams que o necessario, precisa acertar o tamanho do bloco na ultima stream
		// essa ultima stream pega apenas o restante para processas; nao o vlr cheios de threadsPerGrid
		if((tam-i)< threadsPerGrid)
			threadsPerGrid = tam - i;
			
        //copia um bloco de tamanho threadsPerGrid do vetor A do host para o device
        cudaMemcpyAsync(vetorA_d,vetorA+i, threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream);
		
        //copia um bloco de tamanho threadsPerGrid do vetor B do host para o device
        cudaMemcpyAsync(vetorB_d,vetorB+i, threadsPerGrid*(sizeof(int)),cudaMemcpyHostToDevice,stream);
		
        //Invoca o kernel soma passando a stream como argumento
        soma <<<blocksPerGrid,threadsPerBlock,0,stream>>> (vetorA_d,vetorB_d,vetorC_d,threadsPerGrid,i);
		// <<<4,2>> para alguns dos exemplos acima.
		
        //Copia um bloco de tamanho threadsPerGrid do resultado de volta para o host
        cudaMemcpyAsync(vetorC+i,vetorC_d,threadsPerGrid*(sizeof(int)),cudaMemcpyDeviceToHost,stream);
		
    }
    //Sincroniza a stream
    cudaStreamSynchronize(stream);

    //Imprime o resultado no host
    for(i=0;i<tam;i++)
	{
       printf("%d ",vetorC[i]);
    }
    
    //Desaloca os vetores no host
    cudaFreeHost(vetorA);
    cudaFreeHost(vetorB);
    cudaFreeHost(vetorC); 
    
    //Desaloca os vetores no device
    cudaFree(vetorA_d);
    cudaFree(vetorB_d);
    cudaFree(vetorC_d);
    
    //Destroi a stream
    cudaStreamDestroy(stream);
}