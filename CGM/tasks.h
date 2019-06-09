#include <stdlib.h>
#include <stdio.h>  //remove later
#include "periodic.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>


//contains periodic threads functions and global variables

#define LIST_SIZE 50     //size of circular buffer that contains the last read glucose values
#define BUFFER_SIZE 250     //size of socket messa buffers
#define MEAN_SAMPLES  5           //number of samples that periodic_mean will use **ATTENTION!!** must be less than LIST_SIZE

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;         //mutex to acess glucose_list vector and index_list
pthread_cond_t  list_cond  = PTHREAD_COND_INITIALIZER;          //cond to access glucose list       TODO: implement access using conditional variables
pthread_mutex_t message_stack_mutex =PTHREAD_MUTEX_INITIALIZER; //mutex to access message stack
pthread_cond_t  message_stack_cond = PTHREAD_COND_INITIALIZER; //cond to access message stack

int read_sensor();
void *write_socket(void *arg);
void *comand_handler(void * arg);
void *periodic_reading(void *arg);
void *periodic_mean(void *arg);
void *periodic_check_levels(void *arg);
void execute_comands(int cmd, int value);
int parse_command(char * str);
void set_high(int value);
void set_low(int value);
void read_mean();
void read_last();
void print_usage();

static int glucose_list[LIST_SIZE] = {0};
static int index_list = 0;
static int list_mean;
static int hyperglucose_limit = 180;
static int hypoglucose_limit = 70;
static int message_available = 0;           // is 1 when a message is available
static char messages[BUFFER_SIZE] = {0};    //stores message buffer


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
            //printf("periodic_reading():\t glucose_list[index_list]=%d\tindex_list=%d\n", glucose_list[index_list], index_list);
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
            for(int i = index_list; i > (index_list - MEAN_SAMPLES); i--)
            {
                list[i] = glucose_list[i];
            }
        pthread_mutex_unlock(&list_mutex);
        //calculate mean value
        mean = 0;
        for(int i = 0; i < MEAN_SAMPLES; i++)
            mean += (list[i] / MEAN_SAMPLES);
        
        //MUTEX HERE!
        list_mean = mean;       //make mean visible globaly
        //printf("periodic_mean=%d\n", mean);
        wait_period(&info);
    }
}

//periodic task that checks if the last reading was a hypoglucose or hiperglucose
void *periodic_check_levels(void *arg)
{
    //checks last mean and checks if it is a hipoglucose or a hiperglucose level.
}


//non periodic tasks resposible for reading the socket commands and handling commands sent via socket

//parses a NULL terminated string and return an integer associated with the parsed command
//executes comand and returns integer corresponding to executed command
//returns -1 on failure and prints usage
//returns 1 for parsed "set high" and sets *value to <value>
//returns 2 for parsed "set low" and sets  *value to <value>
//returns 3 for parsed "read mean"
//returns 4 for parsed "read last"
int parse_command(char * str)
{
    int value = 0, cmd;
    char * arg1 = strsep(&str, " \n\t");
    char * arg2 = strsep(&str, " \n\t");
    if(strcmp(arg1, "set") == 0)
    {
        if(str != NULL)
            value = atoi(str);
        if(strcmp(arg2, "low") == 0 && value > 0)
        {
            execute_comands(1, value);
            return 1;
        }
        else if(strcmp(arg2, "high") == 0 && value > 0)
        {
            execute_comands(2, value);
            return 2;
        }
        else
        {
            print_usage();
            return -1;
        }
    }
    else if(strcmp(arg1, "read") == 0)
    {
        if(strcmp(arg2, "mean") == 0)
        {
            execute_comands(3, value);
            return 3;
        }
        else if(strcmp(arg2, "last") == 0)
        {
            return 4;
        }
        else
        {
            print_usage();
            return -1;
        }
    }
    else
    {
        print_usage();
        return -1;
    }
}



void execute_comands(int cmd, int value)
{
    switch(cmd)
    {
        case 1:
            set_high(value);
            break;
        case 2:
            set_low(value);
            break;
        case 3:
            read_mean();
            break;
        case 4:
            read_last();
            break;
    }
}


void *comand_handler(void * arg)
{
    int sockfd = (int) arg;
    char buffer[50];
    int bytes_transferred;

    while(1)
    {
        printf("Waiting for command:\n");
        memset(buffer, 0, sizeof(buffer));
        bytes_transferred = read(sockfd,buffer,50);
        if(bytes_transferred <= 0)
        {
            close(sockfd);
            printf("Server disconnected...\n");
            exit(-1);
        }
        printf("Recieved: %s\n", buffer);
        parse_command(buffer);
    }
}


void *write_socket(void *arg)
{
    int sockfd = (int) arg;
    int bytes_transferred;
    char buffer[BUFFER_SIZE];
    while(1)
    {
        pthread_mutex_lock(&message_stack_mutex);
        while(message_available <= 0)
        {
            pthread_cond_wait(&message_stack_cond, &message_stack_mutex);
        }

        //get last message from stack
        strcpy(buffer, messages);
        message_available--;
        //send it to socket
        bytes_transferred = write(sockfd, buffer, BUFFER_SIZE);
        if(bytes_transferred <= 0)
        {
            close(sockfd);
            printf("Server disconnected...\n");
            exit(-1);
        }
        pthread_cond_broadcast(&message_stack_cond);
        pthread_mutex_unlock(&message_stack_mutex);
    }
}


//todo: implement this functions
void set_high(int value)
{
    printf("Set high to %d\n", value);
}


void set_low(int value)
{
    printf("Set low to %d\n", value);
}


void read_mean()
{
    printf("Mean is: %d\n", 2);
}


void read_last()
{
    printf("Last: %d\n", 2);
}

void print_usage()
{
    printf("usage:\n");
    //todo: mostrar como usar
}