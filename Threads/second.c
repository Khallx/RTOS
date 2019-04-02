#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define NT 20


void *wait2sec(void *arg) {
    clockid_t clk;
    struct timespec t;
    if(pthread_getcpuclockid(pthread_self(), &clk))
    {
        printf("Erro em achar clk id de thread[%ld]\n", pthread_self());
        pthread_exit(NULL);
    }
    clock_gettime(clk, &t);
    while(t.tv_sec < 10)
    {
        clock_gettime(clk, &t);             //waits for 2 seconds
    }
    printf("Thread[%ld]\t %ld\n", pthread_self(), t.tv_sec);
    pthread_exit(NULL);
}


int main()
{
    pthread_t thread[NT];

    for(long i = 0; i < NT; i++)
    {
        if(pthread_create(&thread[i], NULL, wait2sec, NULL))
        {
            printf("Erro na criação de thread %ld\n", i);
            return -1;
        }
    }
    //TODO: ADICIONAR ESCALONAMENTO E PRIORIDADE
    //TODO: ADICIONAR CPUS
    for(int i = 0; i < NT; i++)
        pthread_join(thread[i], NULL);
    return 0;
}