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

typedef struct nodo {
    int newsockfd;
    int porta;
    char ip[12];
    struct nodo *next;
}node_t;

typedef struct argument
{
    node_t *head;
    node_t *current;
}argument_t;

// para cada nodo: socket de conexão (obtido através de um accept), porta desse nodo, ip desse nodo


node_t *Head = NULL;

int cancel_connection(node_t *head);
void *server(void *arg);
int new_connection(node_t *head, int sockfd, struct sockaddr * const cli_addr, socklen_t * const clilen);


int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int sockfd, portno;


    if (argc < 2) {
        printf("Erro, porta nao definida!\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Erro abrindo o socket!\n");
        exit(1);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));

    portno = atoi(argv[1]);
    if(portno < 1024 || portno > 49151)
    {
        printf("Número da porta deve estar entre 1024 e 49151\n");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro fazendo bind!\n");
        exit(1);
    }

    //escuta até 5 conexões
    listen(sockfd,5);

    while (1) {
        new_connection(Head, sockfd,(struct sockaddr *) &cli_addr, &clilen);
    }

    //    close(newsockfd);
    //    close(sockfd);
    return 0; 
}


void *server(void *arg) {
    node_t *head = ((argument_t *) arg)->head;                                  //contains address the current iteration of node
    node_t *const current = ((argument_t *) arg)->current;                            //contains address the thread's node
    int i, n;
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("antes do memset:\n");
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
            if (head != current) {
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


int new_connection(node_t *head, int sockfd, struct sockaddr * cli_addr, socklen_t * clilen)
{
    pthread_t new_server;
    node_t *first_node = head;
    argument_t arg;

    int newsk = accept(sockfd,(struct sockaddr *) cli_addr,clilen);
    if(newsk < 0)
    {
        return -1;
    };
    printf("entrando no mutex\n");
    pthread_mutex_lock(&m);     //mutex to create new node:
    while(head != NULL)
    {
        head->next;
    }
    head = malloc(sizeof(node_t));
    head->newsockfd = newsk;
    head->next = NULL;
    printf("criando thread:\n");
    arg.head = first_node;
    arg.current = head;
    pthread_create(&new_server, NULL, server, (void *) &arg);
    pthread_mutex_unlock(&m);
}


int cancel_connection(node_t *head)
{
    //todo: destruir o nodo, reconectar os antigos
}


