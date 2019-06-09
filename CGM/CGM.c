#include "tasks.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>		// inet_aton
#include <netdb.h> 

/*
    simulates a continuos glucose test reading randomized data
    comunicates with a terminal to recieve comands and send information
*/


int main(int argc, char *argv[]) 
{
    pthread_t mean, read, check, cmd, wr_sockt;
    int port_number, sockfd;
    struct sockaddr_in serv_addr;
    sigset_t alarm_sig;
    
    printf("Continuos Glucouse Monitoring Simulator - CGMS\n");
    if(argc < 3)
    {
        printf("Error: please define IP address and port number\n");
        printf("Usage:\n\t%s IP_number Port_number\n", argv[0]);
        printf("\t\tIP_number must be x.x.x.x\n\t\tPort_number must be between 1024 and 49151\n");
        exit(-1);
    }

    //configure socket
    port_number = atoi(argv[2]);
    if(port_number < 1024 || port_number > 49151)
    {
        printf("Error: port number is not in range of 1024 to 49151\n");
        exit(-1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Error: could not create a new socket!\n");
        return -1;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_aton(argv[1], &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port_number);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        printf("Error: could not connect to %s on port %s\n", argv[1], argv[2]);
        exit(-1);
    }

    printf("Connected to %s on port %s successfully\n", argv[1], argv[2]);
    
    //configure real time signals
    //Block all real time signals so they can be used for the timers.
    set_periodic_signals();

    if(pthread_create(&cmd, NULL, comand_handler, (void *) sockfd))
    {
        printf("Error creating comand_handler task\n");
        return -1;
    }

    if(pthread_create(&wr_sockt, NULL, write_socket, (void *) sockfd))
    {
        printf("Error creating write_socket task\n");
        return -1;
    }



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