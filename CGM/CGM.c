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


//returns blood glucose value read from sensor in mg/dL
//TODO: mudar isso pra ser dependente te 2 parametros: a cada x leituras o usuario usa insulina ou se alimenta
//fazendo um incremento e decremento constante apartir disso
int read_sensor()
{
    static int blood_glucose = 100;                  //initial glucose level is 100 mg/dl
    static int food_counter, insulin_counter, index;  // counts the number of reading food and insulin will act upon blood sugar
    
    srand(blood_glucose + food_counter + index);
    if(index == 0)
    {
        food_counter += rand() % 5;
    }
    else if(index == 5)
    {
        insulin_counter += rand() % 5;
    }
    //calculate blood glucose
    if(blood_glucose < 30)
    {
        blood_glucose += rand() % 20 + 20;
        food_counter++;         //to compensate the low blood sugar
    } 
    else if(blood_glucose > 400)
    {
        blood_glucose -= (rand() % 20 + 20);
        insulin_counter++;      //to compensate the high blood sugar
    }
    else
    {
       blood_glucose += (food_counter*food_counter/8) - (insulin_counter*insulin_counter/8) + (rand()%6 - 3);
    }
    index = (index + 1) % 25;                   //every 25 reading the cicle restarts  
    food_counter -= (food_counter) ? 0 : 1;
    insulin_counter -= (insulin_counter) ? 0 : 1;

    return blood_glucose;
}


int main(int argc, char *argv[]) 
{
    for(int i = 0; i < 500; i++)
        printf("%d\n", read_sensor());
}