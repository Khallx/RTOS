#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define NT 5
//RODAR COMO ADMINSTRADOR

void *wait2sec(void *_tid) {
    long tid = (long) _tid;
    clockid_t clk;
    struct timespec t;
    if(pthread_getcpuclockid(pthread_self(), &clk))
    {
        printf("Erro em achar clk id de thread[%ld]\n", tid);
        pthread_exit(NULL);
    }
    clock_gettime(clk, &t);
    while(t.tv_sec < 2)
    {
        clock_gettime(clk, &t);             //waits for 2 seconds
    }
    printf("exiting thread[%ld]\n", tid);
    pthread_exit(NULL);
}


int main()
{
    pthread_t thread[NT];
    struct sched_param param[NT];
    int max_prioridade, min_prioridade;
    srand(time(NULL));              //genrate random numbers for priority
    int algoritmo = SCHED_FIFO;
    cpu_set_t cpus[NT], setcpu;
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Usando todos os %ld cpus\n", num_cpus);

    max_prioridade = sched_get_priority_max(algoritmo);
    min_prioridade = sched_get_priority_min(algoritmo);

    for(int i = 0; i < NT; i++)
    {
        CPU_SET(i % num_cpus, &cpus[i]);        //sets a cpu for each thread
    }

    for(long i = 0; i < NT; i++)
    {
        if(pthread_create(&thread[i], NULL, wait2sec, (void *) i))
        {
            printf("Erro na criação de thread %ld\n", i);
            return -1;
        }
    }


    for(int i = 0; i < NT; i++)
    {
        //param[i].sched_priority = max_prioridade - rand() % max_prioridade;
        param[i].sched_priority = max_prioridade;
        //printf("Prioridade %d: %d\n", i, param[i].sched_priority);
        if (pthread_setschedparam(thread[i], algoritmo, &param[i]) != 0) {
            printf("Erro pthread_setschedparam %d!\n", i);
            printf("Tente novamente com privilégios de administrador\n");
            return -2;
        }
    }

    for(int i = 0; i < NT; i++)
    {
        printf("Thread[%d]:\tafinidade: %ld\tprioridade: %d\n", i, i % num_cpus, param[i].sched_priority);
        pthread_setaffinity_np(thread[i], sizeof(cpus[i]), &cpus[i]);
    }
    for(int i = 0; i < NT; i++)
    {
        pthread_join(thread[i], NULL);
        //printf("saindo da thread[%i]\tprioridade: %d\n", i, param[i].sched_priority);
    }
    return 0;
}