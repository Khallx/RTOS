#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void *read_socket(void * arg);
void *write_socket(void * arg);


int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int sockfd, portnumber;

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

    //fazer accept da conexão aqui
    //criar uma thread que le o socket
    //criar uma thread que escreve no socket baseado no que ela recebe da linha de comando
}