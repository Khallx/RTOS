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

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;         //mutex to acess glucose_list vector and list_index
pthread_cond_t  list_cond  = PTHREAD_COND_INITIALIZER;          //cond to access glucose list       TODO: implement access using conditional variables
pthread_mutex_t message_stack_mutex =PTHREAD_MUTEX_INITIALIZER; //mutex to access message stack
pthread_cond_t  message_stack_cond = PTHREAD_COND_INITIALIZER; //cond to access message stack
pthread_mutex_t mean_mutex = PTHREAD_MUTEX_INITIALIZER;         //mutex to access list_mean
pthread_mutex_t limit_mutex = PTHREAD_MUTEX_INITIALIZER;        // mutex to access glucose limits variables

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
void send_message(char * string);

static int glucose_list[LIST_SIZE] = {0};
static int list_index = 0;
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
    index = (index + 1) % 25;                   //every 25 readings the cicle restarts  
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
            glucose_list[list_index] = read_sensor();
            //printf("periodic_reading():\t glucose_list[list_index]=%d\tlist_index=%d\n", glucose_list[list_index], list_index);
            list_index = (list_index + 1) % LIST_SIZE;       //circular buffer.
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
    int i;
    while(1)
    {
        pthread_mutex_lock(&list_mutex);
        //read the current available readings
        if(list_index - MEAN_SAMPLES < 0)
        {
            i = list_index - MEAN_SAMPLES + LIST_SIZE;
        }
        else
        {
            i = list_index - MEAN_SAMPLES;
        }
        
        for(; i < list_index; i = (i+1) % LIST_SIZE)
        {
            list[i] = glucose_list[i];
        }
        pthread_mutex_unlock(&list_mutex);
        //calculate mean value
        mean = 0;
        for(i = 0; i < MEAN_SAMPLES; i++)
            mean += (list[i] / MEAN_SAMPLES);
        
        pthread_mutex_lock(&mean_mutex);
        list_mean = mean;       //make mean visible globaly
        pthread_mutex_unlock(&mean_mutex);
        wait_period(&info);
    }
}

//periodic task that checks if the last mean was a hypoglucose or hiperglucose
void *periodic_check_levels(void *arg)
{
    struct periodic_info info;
	make_periodic (30000000, &info);      //period of 30s
    int mean, hypo, hyper;
    //checks last mean and checks if it is a hipoglucose or a hiperglucose level.
    while(1)
    {
        pthread_mutex_lock(&mean_mutex);
        mean = list_mean;
        pthread_mutex_unlock(&mean_mutex);
        pthread_mutex_lock(&limit_mutex);
        hypo = hypoglucose_limit;
        hyper = hyperglucose_limit;
        pthread_mutex_unlock(&limit_mutex);
        if(mean < hypo && mean != 0)
        {
            //snprintf(string, 50, "Hypoglucose value: %d", mean);
            printf("Hipoglucose value of %d\n", mean);
        }
        else if(mean > hyper)
        {
            printf("Hyperglucose of %d\n", mean);
        }
        wait_period(&info);
    }
}


//non periodic tasks resposible for reading the socket commands and handling commands sent via socket

//parses a NULL terminated string and return an integer associated with the parsed command
//executes comand and returns integer corresponding to executed command
//returns -1 on failure and prints usage
//returns 1 for parsed "set high"
//returns 2 for parsed "set low" 
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
            set_low(value);
            return 1;
        }
        else if(strcmp(arg2, "high") == 0 && value > 0)
        {
            set_high(value);
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
            read_mean();
            return 3;
        }
        else if(strcmp(arg2, "last") == 0)
        {
            read_last();
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


void *comand_handler(void * arg)
{
    int sockfd = (int) arg;
    char buffer[BUFFER_SIZE];
    int bytes_transferred;

    while(1)
    {
        printf("Waiting for command:\n");
        memset(buffer, 0, sizeof(buffer));
        bytes_transferred = read(sockfd,buffer,BUFFER_SIZE);
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
        printf("Writing to socket: %s\n", buffer);
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



void set_high(int value)
{
    char string[BUFFER_SIZE];
    pthread_mutex_lock(&limit_mutex);
    if(hypoglucose_limit < value)
    {
        hyperglucose_limit = value;         //hyperglucose_limit must always be higher than hypoglucose_limit
        snprintf(string, BUFFER_SIZE, "Hyperglucose limit: %d\n", hyperglucose_limit);
    }
    else
    {
        snprintf(string, BUFFER_SIZE, "Value lower than hypoglucose, unchanged from: %d\n", hyperglucose_limit);
    } 
    pthread_mutex_unlock(&limit_mutex);
    send_message(string);
}


void set_low(int value)
{
    char string[BUFFER_SIZE];
    pthread_mutex_lock(&limit_mutex);
    if(hyperglucose_limit > value)
    {
        hypoglucose_limit = value;         //hypoglucose_limit must always be lower than hyperglucose_limit
        snprintf(string, BUFFER_SIZE, "Hypoglucose limit: %d\n", hypoglucose_limit);
    }
    else
    {
        snprintf(string, BUFFER_SIZE, "Value higher than hyperglucose, unchanged from: %d\n", hypoglucose_limit);
    } 
    pthread_mutex_unlock(&limit_mutex);
    send_message(string);
}


void read_mean()
{
    char string[BUFFER_SIZE];
    pthread_mutex_lock(&mean_mutex);
    snprintf(string, BUFFER_SIZE, "Current mean: %d\n", list_mean);
    pthread_mutex_unlock(&mean_mutex);
    send_message(string);
}


void read_last()
{
    char string[BUFFER_SIZE];
    pthread_mutex_lock(&list_mutex);
    snprintf(string, BUFFER_SIZE, "Last read value: %d\n", glucose_list[list_index]);
    pthread_mutex_unlock(&list_mutex);
    send_message(string);
}


void print_usage()
{
    send_message("Wrong usage\n");
}


//sends string to message queue
void send_message(char * string)
{
    pthread_mutex_lock(&message_stack_mutex);
    while(message_available > 0)
    {
        pthread_cond_wait(&message_stack_cond, &message_stack_mutex);
    }
    message_available++;
    strcpy(messages, string);
    pthread_cond_broadcast(&message_stack_cond);
    pthread_mutex_unlock(&message_stack_mutex);
}