#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

// Número total de clientes, caixas e capacidade máxima de cada fila
#define N_CLIENTES 10
#define N_CAIXAS 2
#define TAM_FILA 3 

// Semáforos para sinalizar quando há cliente na fila e quando o atendimento foi concluído
sem_t sem_cliente_na_fila[N_CAIXAS];
sem_t sem_atendido[N_CAIXAS];

// Contadores de clientes nas filas de cada caixa
int filas[N_CAIXAS];
pthread_mutex_t mutex_filas; // Protege acesso à array de filas

bool encerrar = false; // Flag para encerrar os caixas ao final

bool caixa_status[N_CAIXAS] = {false};  // true=atendendo, false=livre

// Variáveis para atribuir IDs aos clientes na ordem de chegada real
int prox_id_chegada = 0;
pthread_mutex_t mutex_id; // Protege acesso à geração de ID

pthread_mutex_t mutex_status = PTHREAD_MUTEX_INITIALIZER;


// Função para imprimir visualmente o estado das filas
void imprimir_estado() {
    pthread_mutex_lock(&mutex_filas);
    pthread_mutex_lock(&mutex_status);
    
    system("clear");
    printf("╔════════════════════════════╗\n");
    printf("║      MERCADO SIMULADOR     ║\n");
    printf("╠════════════════════════════╣\n");
    
    // Print cashier status and queues
    for (int i = 0; i < N_CAIXAS; i++) {
        printf("║ Caixa %d: %s Fila: [", i, 
               caixa_status[i] ? "[AT]" : "[--]");
        for (int j = 0; j < filas[i]; j++) printf("#");
        for (int j = filas[i]; j < TAM_FILA; j++) printf(".");
        printf("] (%d/%d) ║\n", filas[i], TAM_FILA);
    }
    
 
    
    printf("╚════════════════════════════╝\n");
    
    sleep(0.5); // 1s taxa de atualização
    pthread_mutex_unlock(&mutex_status);
    pthread_mutex_unlock(&mutex_filas);
}


void* f_caixa(void* v) {
    int id = (int)v;

    while (1) {
        if (encerrar) break; // Encerra a thread quando o trabalho termina

        // Tenta pegar um cliente da fila (semáforo)
        if (sem_trywait(&sem_cliente_na_fila[id]) == 0) {
            pthread_mutex_lock(&mutex_filas);
            if (filas[id] > 0) filas[id]--; // Remove cliente da fila

            // Atalizar status do caixa
            pthread_mutex_lock(&mutex_status);
            caixa_status[id] = true;
            pthread_mutex_unlock(&mutex_status);


            pthread_mutex_unlock(&mutex_filas);

            imprimir_estado(); // Atualiza visual após retirada da fila

            // Atendimento (simulado com sleep)
            printf("Caixa %d está atendendo um cliente.\n", id);
            sleep(1 + rand() % 3); // Tempo de atendimento: 1 a 3 segundos
            printf("Caixa %d terminou de atender um cliente.\n", id);

            // Marca o serviço completo
            pthread_mutex_lock(&mutex_status);
            caixa_status[id] = false;
            pthread_mutex_unlock(&mutex_status);

            sem_post(&sem_atendido[id]); // Sinaliza ao cliente que foi atendido
            imprimir_estado();
        } else {
            usleep(10000); // Espera curta para evitar busy waiting
        }
    }

    printf("Caixa %d encerrando...\n", id);
    return NULL;
}


void* f_cliente(void* v) {
    // Espera um tempo aleatório para simular chegada ao mercado
    sleep(1 + rand()%3); // Geração de cliente novo: 1 a 3 segungos
    // Pega um ID único por ordem de chegada
    pthread_mutex_lock(&mutex_id);
    int meu_id = prox_id_chegada++;
    pthread_mutex_unlock(&mutex_id);

    int meu_caixa = -1;

    // Procura uma fila com espaço (preferência para a menor fila disponível)
    pthread_mutex_lock(&mutex_filas);
    int achou_fila = 0;
    for (int i = 0; i < N_CAIXAS; i++) {
        if (filas[i] < TAM_FILA) {
            if (meu_caixa == -1 || filas[i] < filas[meu_caixa]) {
                meu_caixa = i;
                achou_fila = 1;
            }
        }
    }

    if (achou_fila) {
        filas[meu_caixa]++; // Entra na fila escolhida
        pthread_mutex_unlock(&mutex_filas);

        printf("Cliente %d entrou no mercado e foi para a fila do caixa %d.\n", meu_id, meu_caixa);
        imprimir_estado(); // Atualiza visual após entrada na fila

        sem_post(&sem_cliente_na_fila[meu_caixa]);  // Notifica o caixa da sua chegada
        sem_wait(&sem_atendido[meu_caixa]);         // Espera ser atendido

        printf("Cliente %d foi atendido e saiu do mercado.\n", meu_id);
        imprimir_estado();
    } else {
        pthread_mutex_unlock(&mutex_filas);
        printf("Cliente %d não entrou no mercado (todas as filas cheias).\n", meu_id);
        imprimir_estado();
    }

    return NULL;
}


int main() {
    pthread_t thr_clientes[N_CLIENTES], thr_caixas[N_CAIXAS];
    int id_caixas[N_CAIXAS];

    // Inicializa mutexes
    pthread_mutex_init(&mutex_filas, NULL);
    pthread_mutex_init(&mutex_id, NULL);

    // Inicializa semáforos e cria threads dos caixas
    for (int i = 0; i < N_CAIXAS; i++) {
        sem_init(&sem_cliente_na_fila[i], 0, 0);
        sem_init(&sem_atendido[i], 0, 0);
        filas[i] = 0;
        id_caixas[i] = i;
        pthread_create(&thr_caixas[i], NULL, f_caixa, &id_caixas[i]);
    }

    // Cria threads dos clientes com um intervalo entre eles
    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_create(&thr_clientes[i], NULL, f_cliente, NULL);
        usleep(300000 + rand() % 1000000);  // 0.3s a 1.3s entre chegadas
    }

    // Espera todos os clientes terminarem
    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_join(thr_clientes[i], NULL);
    }

    // Sinaliza que é hora dos caixas encerraram e os "acorda" se estiverem esperando
    encerrar = true;
    usleep(50000);
    for (int i = 0; i < N_CAIXAS; i++) {
        sem_post(&sem_cliente_na_fila[i]); // Garante que o caixa não fique travado esperando cliente
        pthread_join(thr_caixas[i], NULL);
    }

    printf("Todos os caixas encerraram.\n");

    return 0;
}

