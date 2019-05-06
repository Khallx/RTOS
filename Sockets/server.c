#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


#define BUFFER_SIZE 256
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

struct nodo {
    int newsockfd;
    int porta;
    char ip[12];
    struct nodo *next;
};

typedef struct argument
{
    struct nodo *head;
    struct nodo *current;
}argument_t;

// para cada nodo: socket de conexão (obtido através de um accept), porta desse nodo, ip desse nodo


struct nodo *Head = NULL;

int cancel_connection(struct nodo *head);
void *server(void *arg);
int new_connection(struct nodo *head, int sockfd, struct sockaddr * const cli_addr, socklen_t * const clilen);


int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int sockfd, portno;
    //     char buffer[256];
    //     int n;
    pthread_t t;
    if (argc < 2) {
        printf("Erro, porta nao definida!\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Erro abrindo o socket!\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro fazendo bind!\n");
        exit(1);
    }
    listen(sockfd,5);

    while (1) {
        new_connection(Head, sockfd,(struct sockaddr *) &cli_addr,&clilen);
    }

    //    close(newsockfd);
    //    close(sockfd);
    return 0; 
}


void *server(void *arg) {
    struct nodo *head = ((argument_t *)arg)->head;                                  //contains address the current iteration of node
    struct nodo *current = ((argument_t *)arg)->current;                            //contains address the thread's node
    int i, n;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = read(current->newsockfd,buffer,50);
        printf("Recebeu: %s - %lu\n", buffer,strlen(buffer));
        if (n < 0) {
            printf("Erro lendo do socket!\n");
            cancel_connection(current);             //destroy connection
        }
        // MUTEX LOCK - GERAL
        pthread_mutex_lock(&m);
        while(head != NULL) {
            if (head == current) {
                n = write(head->newsockfd,buffer,50);
                if (n < 0) {
                    printf("Erro escrevendo no socket!\n");
                    cancel_connection(head);
                }
            }
            // COMO LIDAR COM COMANDO SAIR
        }
        pthread_mutex_unlock(&m);
        // MUTEX UNLOCK - GERAL
    }
}


int new_connection(struct nodo *head, int sockfd, struct sockaddr * cli_addr, socklen_t * clilen)
{
    pthread_t new_server;
    struct nodo *first_node = head;
    int newsk = accept(sockfd,(struct sockaddr *) cli_addr,clilen);
    if(newsk < 0)
    {
        return -1;
    };

    pthread_mutex_lock(&m);     //mutex to create new node:
    while(head != NULL)
    {
        head->next;
    }
    head = malloc(sizeof(struct nodo));
    head->next = NULL;
  
    pthread_create(&new_server, NULL, server, (void *)first_node);
    pthread_mutex_unlock(&m);
}


int cancel_connection(struct nodo *head)
{
    //todo: destruir o nodo, reconectar os antigos
}


