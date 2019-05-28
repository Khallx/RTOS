#include "periodic.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define LIST_SIZE 50     //size of circular buffer that contains the last read glucose values

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;         //mutex to acess glucose_list vector and index_list
/*
    simulates a continuos glucose test reading randomized data
    comunicates with a terminal to 
    TODO: explaing what this does
*/

static int glucose_list[LIST_SIZE] = {0};
static int index_list = 0;
static int list_mean;

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


void *periodic_reading(void *arg)
{
    struct periodic_info info;
	make_periodic (500000, &info);      //period of 500 ms

    while(1)
    {
        pthread_mutex_lock(&list_mutex);
            glucose_list[index_list] = read_sensor();
            index_list = (index_list + 1) % LIST_SIZE;       //circular buffer.
        pthread_mutex_unlock(&list_mutex);
        wait_period(&info);
    }
}


void *periodic_mean(void *arg)
{
    struct periodic_info info;
	make_periodic (1000000, &info);      //period of 1 s
    int current_index;
    int list[LIST_SIZE];
    int mean;
    while(1)
    {
        pthread_mutex_lock(&list_mutex);
            //read the current available readings
            for(int i = 0; i < LIST_SIZE; i++)
            {
                list[i] = glucose_list[i];
            }
        pthread_mutex_unlock(&list_mutex);
        //calculate mean value
        for(int i = 0; i < LIST_SIZE; i++)
            mean += (list[i] / LIST_SIZE);
        list_mean = mean;       //make mean visible globaly
    }
}


int main(int argc, char *argv[]) 
{

}