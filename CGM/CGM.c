#include "tasks.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

/*
    simulates a continuos glucose test reading randomized data
    comunicates with a terminal to recieve comands and send information
*/


int main(int argc, char *argv[]) 
{
    pthread_t mean, read, check;
    sigset_t alarm_sig;
    printf("Continuos Glucouse Monitoring Simulator - CGMS\n");
    if(argc < 3)
    {
        printf("Error: please define IP address and port number\n");
        printf("Usage:\n\t%s IP_number Port_number\n", argv[0]);
        printf("\t\tIP_number must be x.x.x.x\n\t\tPort_number must be between 1024 and 49151\n");
        exit(-1);
    }
    //TODO:
    //configurar o socket
    //tentar se conectar, se falhar simplesmente sair
    //dps de conectado passar o socket a uma thread que recebe comandos

    //configure real time signals
    //Block all real time signals so they can be used for the timers.
    set_periodic_signals();

    //create the 3 periodic tasks
    if(pthread_create(&read, NULL, periodic_reading, NULL))
    {
        printf("Error creating periodic_read task\n");
        return -1;
    }
    if(pthread_create(&mean, NULL, periodic_mean, NULL))
    {
        printf("Error creating periodic_mean task\n");
        return -1;
    }
    if(pthread_create(&check, NULL, periodic_check_levels, NULL))
    {
        printf("Error creating periodic_check task\n");
        return -1;
    }

    pthread_exit(NULL);
}