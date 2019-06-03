#include <stdlib.h>
#include <stdio.h>  //remove later
#include "periodic.h"
#include <pthread.h>
#include <time.h>


//contains periodic threads functions and global variables

#define LIST_SIZE 50     //size of circular buffer that contains the last read glucose values

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;         //mutex to acess glucose_list vector and index_list
pthread_cond_t  list_cond  = PTHREAD_COND_INITIALIZER;          //cond to access glucose list       TODO: implement access using conditional variables


static int glucose_list[LIST_SIZE] = {0};
static int index_list = 0;
static int list_mean;
static int hyperglucose_limit = 180;
static int hypoglucose_limit = 70;


//helper function that simulates the sensor
//returns blood glucose value read from sensor in mg/dL
int read_sensor()
{
    static int blood_glucose = 100;                  //initial glucose level is 100 mg/dl
    static int food_counter, insulin_counter, index;  // counts the number of reading food and insulin will act upon blood sugar
    
    srand(blood_glucose + food_counter + index);        //TODO : change seed
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


//these 3 functions are to be threads:
//periodic task that reads data from sensor
void *periodic_reading(void *arg)
{
    struct periodic_info info;
	make_periodic (500000, &info);      //period of 500 ms

    while(1)
    {
        pthread_mutex_lock(&list_mutex);
            glucose_list[index_list] = read_sensor();
            //remove:
            printf("periodic_reading():\t glucose_list[index_list]=%d\tindex_list=%d\n", glucose_list[index_list], index_list);
            index_list = (index_list + 1) % LIST_SIZE;       //circular buffer.
        pthread_mutex_unlock(&list_mutex);
        wait_period(&info);
    }
}

//periodic task that calculates the mean from the last readings
void *periodic_mean(void *arg)
{
    struct periodic_info info;
	make_periodic (1000000, &info);      //period of 1 s
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
        
        //MUTEX HERE!
        list_mean = mean;       //make mean visible globaly
        printf("periodic_mean=%d\n", mean);
        wait_period(&info);
    }
}

//periodic task that checks if the last reading was a hypoglucose or hiperglucose
void *periodic_check_levels(void *arg)
{
    //checks last mean and checks if it is a hipoglucose or a hiperglucose level.
}


//non periodic tasks resposible for reading the socket commands and handling commands sent via socket

void *socket_handler(void * arg)
{
    //reads and writes to socket as specified
}

void *comand_handler(void * arg)
{

}