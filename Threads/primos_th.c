#include <pthread.h>
#include <stdio.h>

#define NT 10
#define TVP 10000
#define RANGE 10000
long total_primos = 0;          //final value must be  9592 
long numeros_primos[TVP];

void *primos(void *_tid)
{
    long tid = (long) _tid;
    long VI = tid * RANGE;       //initial value
    long VF = VI + RANGE - 1;         //final value
    if(VI == 0)
    {
        VI = 2;                 // 0 e 1 aren't primes
    }
    int is_primo;               //flag for prime detection

    printf("Executando thread id %ld com VI = %ld e VF = %ld\n", tid, VI, VF);
    for(long j = VI; j < VF; j++)
    {
        is_primo = 1;
        for(long i = 2; i <= ((j >> 1)+1); i++)     // max range is VI / 2
        {
            if(j % i == 0)
            {
                is_primo = 0;
                break;
            }
        }
        if(is_primo)
        {
            numeros_primos[total_primos] = j;   
            total_primos++;
        }
    }
    pthread_exit(NULL);
}


int main()
{
    pthread_t th[NT];

    for(long i = 0; i < NT; i ++)
    {
        if(pthread_create(&th[i], NULL, primos, (void*) i))
        {
            printf("Error creating thread %ld\n", i);
            return -1;
        }
    }


    for(long i = 0; i < NT; i ++)
    {
        pthread_join(th[i], NULL);
    }

    for(long i = 0; i < total_primos; i++)
    {
        printf("primo[%ld] = %ld\n", i, numeros_primos[i]);
    }

    printf("O numero de primos eh: %ld\n", total_primos);
    return 0;
}
