/*
Faz a soma dos elementos de dois vetores 

Exemplifica o uso de cudaMallocHost() para alocar memoria paginada no host e
o uso de cudaFreeHost para desalocar()

Para compilar: nvcc 01-soma-vet-pinned.cu -o 01-soma-vet-pinned
Para executar: ./01-soma-vet-pinned

OBS: os valores de tamanho do vetor e o conteudo do vetor 
     estao fixos no codigo

*/


#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

__global__ void soma(int *vetorA, int *vetorB,int *vetorC,int tam)
{ 
    int idx = blockDim.x * blockIdx.x + threadIdx.x; 
    if (idx < tam) 
    {
        vetorC[idx]=vetorA[idx]+vetorB[idx];
    }
}

int main(int argc,char **argv)
{
    int i,*vetorA,*vetorB,*vetorC,threadsPerBlock,blocksPerGrid; 
    int *vetorA_d,*vetorB_d,*vetorC_d;

    int tam= 16; // 5000;
    //Define a quantidade de threads por bloco
    threadsPerBlock = 256; 

    //Aloca os vetores no host
    cudaMallocHost((void**)&vetorA,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorB,tam*(sizeof(int))); 
    cudaMallocHost((void**)&vetorC,tam*(sizeof(int))); 

    //Aloca os vetores no device
    cudaMalloc((void**)&vetorA_d,tam*(sizeof(int))); 
    cudaMalloc((void**)&vetorB_d,tam*(sizeof(int))); 
    cudaMalloc((void**)&vetorC_d,tam*(sizeof(int))); 
    
    //Preenche os vetores no host
    for(i=0;i<tam;i++)
	{
        vetorA[i] = i;
        vetorB[i] = 0; //-i;
    }
    
    //Define a quantidade de blocos por grade
    blocksPerGrid=(tam+threadsPerBlock-1)/threadsPerBlock; 

    //Copia o conteúdo dos vetores para o device
    
    cudaMemcpy(vetorA_d,vetorA,tam*(sizeof(int)), cudaMemcpyHostToDevice); 
    cudaMemcpy(vetorB_d,vetorB,tam*(sizeof(int)), cudaMemcpyHostToDevice); 

    //Invoca o kernel com blocksPerGrid blocos e threadsPerBlock threads
    
    soma <<<blocksPerGrid,threadsPerBlock>>> (vetorA_d,vetorB_d,vetorC_d,tam); 

    //Copia o resultado da soma de volta para o host
    cudaMemcpy(vetorC,vetorC_d,tam*(sizeof(int)), cudaMemcpyDeviceToHost);

    //Imprime o resultado no host
    for(i=0;i<tam;i++)
	{
       printf("%d ",vetorC[i]);
    }
	printf("\n");
	
    
    //Desaloca os vetores no host
    cudaFreeHost(vetorA);
    cudaFreeHost(vetorB);
    cudaFreeHost(vetorC); 
    
    //Desaloca os vetores no device
    cudaFree(vetorA_d);
    cudaFree(vetorB_d);
    cudaFree(vetorC_d);
}