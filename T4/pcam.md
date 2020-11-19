# PCAM
* Daniel Sá Barretto Prado Garcia 103734344
* Felipe Guilermmo Santuche Moleiro 10724010
* Laura Alves de Jesus 10801180
* Tiago Marino Silva 10734748

## Particionamento
Após a leitura do vetor Vet, ele é particionado em partes iguais, sendo a menor parte  possivel uma posição unica do vetor e a maior possivel o proprio vetor. Assim, são geradas N tarefas. Cada tarefa é responsável por encontrar o menor valor maior que Vet[k] dentro de sua parte do vetor. 

## Comunicação
A comunicação entre as tarefas é feita através de uma região de memória compartilhada, correspondente aos 3 valores inteiros de 32 bits, que armazenam o menor valor maior que Vet[k] encontrado, o tamanho do vetor e o valor de Vet[k], respectivamente. Além disso, cada tarefa armazena localmente o valor calculado do menor valor maior que Vet[k] em sua parte, entra em uma região crítica para ler e escrever nessa região de memória compartilhada. Nesta região críWtica, ela verifica se o valor do inteiro é menor que o número de letras de sua substring, caso seja, ela atualiza o valor do inteiro para o valor calculado. Ao acessar a região crítica, a tarefa impede que outras tarefas entrem nesta mesma região simultâneamente.

As threads precisam verificar se a variável min está sendo sobrescrita por outra thread

## Aglomeração
Após a leitura da entrada, teremos um vetor com N posiçoes. Iremos dividir essas N posiçoes em T blocos, onde T é o numero de threads sendo executadas n    A aglomeração das N tarefas é feita em  T threads, é responsável por encontrar o menor valor menos que vet[k] dentro do seu sub-vetor.

    
## Mapeamento 
S