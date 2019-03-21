#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 10  //number of tasks
#define SEED 248  //seed for random numbers
#define PERIODOMAX 251

typedef struct _tarefa
{
    double periodo;
    double comp;           //periodo da Tarefa e tempo de computação
} Tarefa;


int compara(const void * a, const void * b)
{
    const Tarefa *_a = (Tarefa *) a;
    const Tarefa *_b = (Tarefa *) b;
    return (_a->periodo - _b->periodo);             //ordenas as tarefas pelo menor período
}


void printTarefas(const Tarefa * task, const size_t size)
{
    for(int i = 0; i < size; i ++)
    {
        printf("tarefa[%d]:\tperiodo: %g\tc: %g\n", i, task[i].periodo, task[i].comp);
    }
}

double CalcUtilReal(const Tarefa * t, const size_t size)
{
    double count = 0;
    for(int i = 0; i < size; i ++)
    {
        count += (t[i].comp /t[i].periodo);             // Ut = sum(Ci/Pi)
    }
    return count;
}


double CalcUtilTeorico(const size_t size)
{
    double n = (double) size;
    return n * (pow(2, (1/n)) - 1);             //calcula limite teorico de Deadline Monotonica //TODO: NAO RECONHECE POW??
}


//TODO: CALCULAR OUTRO LIMITE COM SOMATORIO ITERATIVO

int main()
{
    Tarefa task[SIZE];
    
    srand(SEED);
    for(int i = 0; i < SIZE; i ++)
    {
        task[i].periodo = (double) (rand() % PERIODOMAX) + 1;
        task[i].comp = (double) (rand()  % (int) task[i].periodo) + 1;
    }

    qsort(task, SIZE, sizeof(Tarefa), compara);
    printTarefas(task, SIZE);

    double tempReal = CalcUtilReal(task, SIZE);
    double tempTeorico = CalcUtilTeorico(SIZE);
    //TODO: MOSTRAR RESULTADOS
    return 0;
}