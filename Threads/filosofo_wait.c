#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NF 3            //NF must be bigger than 1

//pthread_mutex_t garfo[NF] = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
char garfo[NF] = {0};       //garfo = 0 disponível, garfo = 1 indísponível
//o mutex de index anterior é o gafo a esquerda
//o mutex ingual ao indice do filosofo representa o garfo a direita e 
void pensar(long i)
{   
    printf("filosofo %ld pensa\n", i);
    sleep(10);   
}


void comer(long i)
{
    printf("filosofo %ld come\n", i);
    sleep(5);
}


void pegar_garfo(long i, int lado)
{
    pthread_mutex_lock(&mutex);
    printf("filosofo tenta %ld pegar o garfo %d\n", i, lado);
    while(garfo[lado] != 0)        //se garfo estiver pego espera
    {
        pthread_cond_wait(&cond, &mutex);       //tenta pegar o garfo de numero n
    }
    garfo[lado] = 1;              //garfo esta indisponível
    pthread_mutex_unlock(&mutex);
}


void largar_garfo(long i, int lado)
{
    garfo[lado] = 0;              //garfo esta disponível
    pthread_cond_broadcast(&cond);     //larga o garfo de numero n
    printf("filosofo %ld larga o garfo %d\n", i, lado);
}


void *filosofar(void * tid)
{
    long i = (long) tid;
    int direita = i;
    int esquerda = i + 1 % NF;
    while(1)
    {
        if(i == 0)  pegar_garfo(i, direita);        //todos começam na esquerda menos o primeiro
        else pegar_garfo(i, esquerda);
        sleep(1);
        if(i == 0) pegar_garfo(i, esquerda);       //pegam o garfo que falta
        else pegar_garfo(i, direita);
        comer(i);
        largar_garfo(i, direita);
        largar_garfo(i, esquerda);
        pensar(i);
    }
}


int main()
{
    pthread_t filosofo[NF];     //cada thread representa um filosofo

    for(long i = 0; i < NF; i++)
    {
        if(pthread_create(&filosofo[i], NULL, filosofar, (void*) i))
        {
            printf("Erro na criação de thread %ld", i);
            return -1;
        }
        //printf("filoso %ld criado\n", i);
    }
    pthread_exit(NULL);
    
}

