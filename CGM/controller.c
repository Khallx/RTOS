#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

/*
    Sends comands to CGM based on comand line inputs
    Commands are:
        set low <value> 
            -sets the value of a hypoglucose as <value>
        set high <value>
            -sets the value of a hyperglucose as <value>
        read mean
            -prints the mean glucose
        read last
            -prints the last read glucose value
        check
            -checks glucose level
*/

pthread_mutex_t accept_com_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t accept_com_cond = PTHREAD_COND_INITIALIZER;
int num_connections = 0;

//closes socket, frees conditional variable to allow new connections
void close_socket(int sockfd)
{
    pthread_mutex_lock(&accept_com_mutex);
    close(sockfd);
    num_connections--;
    pthread_cond_broadcast(&accept_com_cond);
    pthread_mutex_unlock(&accept_com_mutex);
}

void print_usage()
{
    printf("usage:\n");
    //todo: mostrar como usar
}

void *commands(void * arg)
{
    int sockfd = (int) arg;
    char buffer[50];
    int bytes_transferred;

    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, 50, stdin);
        bytes_transferred = send(sockfd, buffer, 50, NULL);
        if(bytes_transferred <= 0)
        {
            close_socket(sockfd);
            pthread_exit(NULL);
        }
        memset(buffer, 0, sizeof(buffer));
        bytes_transferred = recv(sockfd,buffer,50,0);
        if(bytes_transferred <= 0)
        {
            close_socket(sockfd);
            pthread_exit(NULL);
        }
        printf("%s\n", buffer);
    }
}

//parses a NULL terminated string and return an integer associated with the parsed command
//returns -1 on failure and prints usage
//returns 1 for parsed "set high" and sets *value to <value>
//returns 2 for parsed "set low" and sets  *value to <value>
//returns 3 for parsed "read mean"
//returns 4 for parsed "read last"
int parse_command(char * str, int * value)
{
    *value = -1;
    char * arg1 = strsep(&str, " \n\t");
    char * arg2 = strsep(&str, " \n\t");
    if(strcmp(arg1, "set") == 0)
    {
        *value = 0;
        if(str != NULL)
            *value = atoi(str);
        if(strcmp(arg2, "low") == 0 && *value > 0)
        {
            return 1;
        }
        else if(strcmp(arg2, "high") == 0 && *value > 0)
        {
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

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int sockfd, clientsockfd, portnumber;
    pthread_t commander;

    printf("Continuos Glucouse Monitoring Controller - CGMC\n");
    if(argc < 2)
    {
        printf("Error: please define port number\n");
        printf("\t\tUsage: %s port_number\n\t\tport_number must be e between 1024 and 49151\n", argv[0]);
        exit(-1);
    }

    portnumber = atoi(argv[1]);
    if(portnumber < 1024 || portnumber > 49151)
    {
        printf("Error: port number is not in range of 1024 to 49151\n");
        exit(-1);
    }

    // setup server socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error: socket could not be opened\n");
        exit(-1);
    }

    //bind socket
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnumber);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Error: failed to bind socket\n");
        exit(-1);
    }

    if(listen(sockfd, 1) < 0) 
    {
        printf("Error: failed to listen to socket\n");
        exit(-1);
    }
    printf("Listening for 1 connection on port %d...\n", portnumber);

    //waits for a new connection, creates a thread to handle communication,
    //sets the number of connections to +1
    while(1)
    {
        pthread_mutex_lock(&accept_com_mutex);
        while(num_connections != 0)
        {
            pthread_cond_wait(&accept_com_cond, &accept_com_mutex);
        }
        clientsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (clientsockfd < 0) {
            printf("Error: failure on socket accept\n");
            exit(-1);
        }
        if(pthread_create(&commander, NULL, commands, (void *)clientsockfd))
        {
            printf("Error: could not create a new commander thread\n");
            exit(-1);
        }
        num_connections++;
        pthread_mutex_unlock(&accept_com_mutex);
    }

    //should never reach this point
    pthread_exit(NULL);
}