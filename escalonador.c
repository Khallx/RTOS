#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 10  //number of tasks
#define SEED 255  //seed for random numbers
#define PERIODOMAX 1000
#define COMPMAX 25
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
    printf("n %g\n", n);
    return n * (pow(2, (1/n)) - 1);             //calcula limite teorico de Deadline Monotonica //TODO: NAO RECONHECE POW??
}


int analiseUtil(const Tarefa *t, const size_t size)
{
    return (CalcUtilReal(t, size) < CalcUtilTeorico(size));
}
//TODO: CALCULAR OUTRO LIMITE COM SOMATORIO ITERATIVO


/*  R(i+1) = C + sum(ceil(Ri/Pj)*Cj)  para j pertencente ao conjunto de tarefas com maior prioridade  */
double calcResposta(const Tarefa * t, const int k)             //ponteiro para VETOR TAREFA e a posição da tarefa a ser calculada
{
    double r = t[k].comp;       //r é o tempo de resposta
    double last_r;
    double sum;
    while(r < t[k].periodo)
    {
        last_r = r;
        sum = 0;
        for(int j = 0; j < k; j++)
        {
            sum += ceil(last_r/t[j].periodo) * t[j].comp;           //soma de todas tarefas com maior prioridade
        }
        r = t[k].comp + sum;
        if(r == last_r)
        {
            return r;               //retorna tarefa que converviu
        }
    }
    return r;                       //retorna tarefa que estourou periodo
}


int analiseResposta(const Tarefa * t, const size_t size)
{
    double temp;
    printf("tempos de respostas:\n");
    for(int i = 0; i < size; i++)
    {
        temp = calcResposta(t, i);                              //calcula o tempo de cada resposta de cada tarefa
        if(temp < t[i].periodo)
            printf("tarefa[%d]: %g\n", i, temp);
        else
        {
            printf("tarefa[%d]: %g\t estourou o tempo de resposta\n", i, temp);
            return 0;
        }
    }
    return 1;
}

int main()
{
    Tarefa task[SIZE];

    srand(SEED);
    for(int i = 0; i < SIZE; i ++)
    {
        task[i].comp = (double) (rand()  % COMPMAX) + 1;
        task[i].periodo = (double) (rand() % PERIODOMAX) + task[i].comp;

    }

    qsort(task, SIZE, sizeof(Tarefa), compara);
    printTarefas(task, SIZE);

    double tempReal = CalcUtilReal(task, SIZE);
    double tempTeorico = CalcUtilTeorico(SIZE);
    int analise = analiseUtil(task, SIZE);

    printf("real: %g teorico: %g\n", tempReal, tempTeorico, analise);
    if(analise)
        printf("As tarefas são escalonaveis pelo teste de utlização\n");
    else
        printf("As tarefas NÃO são escalonaveis pelo teste de utlização\n");

    if(analiseResposta(task, SIZE))
        printf("Pelo teste de resposta as tarefas são escalonaveis");
    else
        printf("Pelo teste de resposta as tarefas NÃO são escalonaveis");

    return 0;
}
