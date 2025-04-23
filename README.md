# MC504---Multithread
Projeto: Simulador de Mercado Multithread
Projeto desenvolvido com o objetivo de simular um mercado, com a chegada de clientes, gerenciamento do atendimento no caixa, utilizando técnicas não conflitandes multithreads sendo elas semáforos e mutexes.

## Integrantes
Luiz Eduardo Silva Salustriano 183139
Matheus Rufino da Silva 221756

## Descrição
Comportamento de um mercado com alguns caixas(atendem um cliente por vez, com tempo de atendimento variável), clientes (chegam aleatoriamente e entram na fila do caixa mais vazio) e filas limitadas lidando com problemas de concorrência, evitar condições de corrida e deadlocks; inspirado no problema do Barbeiro Dorminhoco e no livro The "Little Book of Semaphores" de Allen B. O modelo inclui uma visualização para acompanhar a simulação em tempo real.

## Ferramentas e conceitos utilizados
- Linguagem C
- Semáforos
- Mutexes

## Como compilar
gcc -pthread supermerdo.c -o out

## Exemplo de visualização
╔════════════════════════════╗
║      MERCADO SIMULADOR     ║
╠════════════════════════════╣
║ Caixa 0: [AT] Fila: [#..] (1/3) ║
║ Caixa 1: [AT] Fila: [...] (0/3) ║
╚════════════════════════════╝

Onde 
- “[--]” = caixa vazio
- “[AT]” = caixa ocupado
- “#” = cliente na fila
- “.” =  espaço vazio na fila

## Vídeo sobre o projeto
[link]

