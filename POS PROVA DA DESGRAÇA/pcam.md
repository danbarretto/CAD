Daniel Sá Barretto Prado Garcia 103734344
Felipe Guilermmo Santuche Moleiro 10724010
Laura Alves de Jesus 10801180
Tiago Marino Silva 10734748

===Particionamento===
O particionamento será feito da seguinte forma, cada tarefa consiste em uma multiplicação de uma posição da matriz A por um elemento da matriz B. Ou seja, dada uma linha da matriz A e uma coluna da matriz B, serão geradas N tarefas(multiplicam posição i da linha A por i da coluna de B) e após o calculo dessas multiplicaçoes é feita uma redução de soma dos resultados de cada uma dessas tarefas para salvar na matriz C. Executa isso para cada permutação de linhas e colunas possiveis, ou seja, para cada posição da matriz C.

===Comunicação===

Cada tarefa recebe posicao i de uma linha de A e de uma coluna de B, portanto A e B ou devem ser globais ou devem ter seus valores enviados para cada tarefa. Aṕos isso se calcula a tarefa, e é necesaria a comunicação com outras tarefas com outros indices( mas sobre a mesma linha e coluna) para fazer a reducao. Por ultimo é necessario guardar o resultado na matriz C (global ou enviar resultado para mestre).

===Aglomeração===

Estamos trabalhando com um cluster com memoria distribuida e em que cada nó do cluster temos uma maquina MIMD com memoria compartilhada. Nossa aglomeração é feita da seguinte forma, iremos gerar P processos em termo do numero de nós existentes no cluster, e iremos dividir a matriz A por P, ou seja, cada nó ira receber N/P linhas da matriz A e a matriz B inteira. Em cada nó do cluster será feita a divisão da matriz B por T, em que T é o numero de threads criadas, ou seja, cada thread do nó vai cuidar de fazer as multiplicaçoes das N/P linhas pelas N/T colunas, e após a execução de todas as threads, retorna o resultado para o nó mestre.

===Mapeamento===

Os P processos são mapeados nos nós do cluster, espera-se que esse valor P seja igual ao numero de nós do cluster para melhor eficiencia, mas caso contrario, podemos utilizar algoritimos como por exemplo a fila circular para mapear os processos nos nós. Ja para cada processo iremos criar T threads, esse valor de T deve ser escolhido pensando no numero de unidades de processamento que existem no cluster, se todos os nós tem o mesmo numero de unidades de processamento, esse valor é decidido facilmente, ja se temos valores diferentes, o melhor é escolher baseado no nó com o maior numero de unidades de processamento, dessa forma não ficamos com nenhuma unidade ociosa. O escalonamento das threads em cada nó é dado pelo sistema operacional.