/*
Faz a soma dos elementos de dois vetores 

Exemplifica o uso de memoria mapeada com cudaHostAlloc() usando
o parametro cudaHostAllocMapped para alocar memoria 
tanto no host quanto no device. Copias entre host e device sao
implicitas, igual aa memoria unificada.
cudaDeviceSynchronize() antes da impressao do resultado se faz
necessaria, caso contrário o resultado deve sair errado.

Para compilar: nvcc 02-soma-vet-mapped.cu -o 02-soma-vet-mapped
Para executar: ./02-soma-vet-mapped

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

    int tam = 16; //5000;
    //Define a quantidade de threads por bloco
    threadsPerBlock = 256; 

    //Aloca os vetores no host e no device (memória mapeada em endereço virtual unificado)
    cudaHostAlloc((void**)&vetorA,tam*(sizeof(int)),cudaHostAllocMapped); 
    cudaHostAlloc((void**)&vetorB,tam*(sizeof(int)),cudaHostAllocMapped); 
    cudaHostAlloc((void**)&vetorC,tam*(sizeof(int)),cudaHostAllocMapped); 
    
    //Preenche os vetores no host
    for(i=0;i<tam;i++)
	{
        vetorA[i]=i;
        vetorB[i]=0;  //-i;
    }
    
    //Define a quantidade de blocos por grade
    blocksPerGrid=(tam+threadsPerBlock-1)/threadsPerBlock; 

    //Invoca o kernel com blocksPerGrid blocos e threadsPerBlock threads
    
    soma <<<blocksPerGrid,threadsPerBlock>>> (vetorA,vetorB,vetorC,tam); 

	cudaDeviceSynchronize();
	
    //Imprime o resultado no host
    for(i=0;i<tam;i++)
	{
       printf("%d ",vetorC[i]);
    }
	printf("\n");
	

    //Desaloca os vetores no host e no device
    cudaFreeHost(vetorA);
    cudaFreeHost(vetorB);
    cudaFreeHost(vetorC);
}