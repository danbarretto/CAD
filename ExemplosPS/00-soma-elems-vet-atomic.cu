/*
Faz a soma global dos elementos de um vetor

Exemplifica o uso de atomicADD()

Para compilar: nvcc 00-soma-elems-vet-atomic.cu -o 00-soma-elems-vet-atomic
Para executar: ./00-soma-elems-vet-atomic

OBS: os valores de tamanho do vetor e o conteudo do vetor 
     estao fixos no codigo

*/

#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

__global__ void soma_elementos(int *vetorA,int *soma,int tam)
{
    //Calcula o índice global da thread
    int idx = threadIdx.x+blockIdx.x*blockDim.x; 
    if (idx < tam) 
	{
        //Faz a soma entre elemento do vetor no índice idx e o conteúdo de soma
        atomicAdd(soma,vetorA[idx]); 
    }  
}

int main(int argc,char **argv)
{
    
    //Declara as variáveis para uso no host
    int i,*vetorA,threadsPerBlock,blocksPerGrid,soma=0; 
    
    //Declara os ponteiros para alocação no device
    int *vetorA_d,*soma_d; 
    
    //Define o tamanho do vetor
    int tam=16; //5000;

    //Define a quantidade de threads por bloco
    threadsPerBlock = 256; 

    //Aloca memoria paginada para o vetor no host
    vetorA=(int *)malloc(tam * sizeof(int)); 

    //Aloca o vetor no device
    cudaMalloc((void**)&vetorA_d,tam*(sizeof(int))); 
    //Aloca uma variável para armazenar a soma dos elementos do vetor
    cudaMalloc((void**)&soma_d,sizeof(int));
    
    //Preenche o vetor no host
    for(i=0;i<tam;i++)
	{
        vetorA[i]=1;
    }
    
    //Define a quantidade de blocos por grade
    blocksPerGrid=(tam+threadsPerBlock-1)/threadsPerBlock; 

    //Copia o conteúdo do vetor para o device
    cudaMemcpy(vetorA_d,vetorA,tam*(sizeof(int)), cudaMemcpyHostToDevice); 

    //Copia o conteúdo de soma para o device (soma_d = 0)
    cudaMemcpy(soma_d,&soma,sizeof(int), cudaMemcpyHostToDevice); 

    //Invoca o kernel com blocksPerGrid blocos e threadsPerBlock threads
    soma_elementos <<<blocksPerGrid,threadsPerBlock>>> (vetorA_d, soma_d, tam);

    //Copia o resultado da soma de volta para o host
    cudaMemcpy(&soma,soma_d,sizeof(int), cudaMemcpyDeviceToHost);

    //Imprime o resultado no host
    printf("%d\n",soma);
    
    //Desaloca o vetor no host
    free(vetorA);  
    
    //Desaloca os vetores no device
    cudaFree(vetorA_d);
    cudaFree(soma_d);
}
