#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#define NP 3 //numero de produtores
#define NC 4 //numero de consumidores
#define BUFF_SIZE 10 


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
int T = 0;                      //indice do buffer atual 
int buffer[BUFF_SIZE] = {0};

void *produzir(void *_tid)
{
    long tid = (long) _tid;
    while(1)
    {   
        pthread_mutex_lock(&mutex);
        while(T == BUFF_SIZE - 1)
        {
            pthread_cond_wait(&cond, &mutex);
  //          usleep(50);  
        }
        buffer[T] = tid;
        printf("produtor %ld produzindo buffer[%d]\n", tid, T); 
        ++T;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }
}


void *consumir(void *_tid)
{
    long tid = (long) _tid;
    while(1)
    {   
        pthread_mutex_lock(&mutex);
        while(T == 0)
        {
            pthread_cond_wait(&cond, &mutex);  
  //          sleep(2);  
        }
        printf("Consumidor %ld consumindo buffer[%d]=%d\n", tid, T, buffer[T]); 
        T--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);     
        usleep(500000);
    }
}


int main(void)
{
    pthread_t produtor[NP];
    pthread_t consumidor[NC];
    
    for(long i = 0; i < NP; i++)
    {
        if(pthread_create(&produtor[i], NULL, produzir, (void *) i))
            printf("Problema na criação de thread %ld do produtor\n", i);
    }
    
    for(long i = 0; i < NC; i++)
    {
        if(pthread_create(&consumidor[i], NULL, consumir, (void *) i))
            printf("Problema na criação de thread %ld do consumidor\n", i);
    }
    pthread_exit(NULL);
}
