#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

void erro(char *msg);

int main(int argc, char *argv[]) {
    char endServer[100];
    int fd;
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    if (argc < 6) {
        printf("Client <proxy ip> <server ip> <proxy port> <server port> <protocol>\n");
        return -1;
    }
    //must run with 6 arguments however proxy and protocol arguments are ignored in the current version
    //gets name of end server
    strcpy(endServer, argv[2]);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        erro("Nao consegui obter endereço");

    //sets up required struct to connect to server
    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[4]));

    //sets up socket and tries connection
    if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
        erro("socket");
    if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
        erro("Connect");

    write(fd, "Hello World!", sizeof("Hello World!"));
    close(fd);
    exit(0);
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
