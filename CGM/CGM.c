#include "periodic.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define RAND_RANGE 20           // value of maximun increment or decrement between each seasor reading

/*
    simulates a continuos glucose test reading randomized data
    comunicates with a terminal to 
    TODO: explaing what this does
*/


//returns blood glucose value read from sensor
//the value read is always +- 20 units from the last reading
//change is randomized and the sensor never reads below 20 mg/ml
//TODO: mudar isso pra ser dependente te 2 parametros: a cada x leituras o usuario usa insulina ou se alimenta
//fazendo um incremento e decremento constante apartir disso
int read_sensor()
{
    static int last_value = 100;        //initial glucose level is 100 mg/dl
    srand(time(NULL));                  //TODO: change seed to a time variable
    last_value += (rand() % (2*RAND_RANGE)) - RAND_RANGE;       // range of increment is -RAND_RANGE to + RAND_RANGE
    //limits reading values to range 20 to 400
    if(last_value < 20)
    {
        last_value = 10 + (rand() % (2*RAND_RANGE));
    }
    else if(last_value > 400)
    {
        last_value = 400 - (rand() % (2*RAND_RANGE));
    }
    return last_value;
}


int main()
{
    for(int i = 0; i < 500; i ++)
    {
        printf("%d\n", read_sensor());
    }
}