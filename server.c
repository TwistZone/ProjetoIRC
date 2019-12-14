/*******************************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surjem
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define BUF_SIZE 1000

pthread_t *threads;
int *client_fds;
char fim;

void *process_client(void *arg);

void erro(char *msg);

int find_empty_slot(int *array, int size);

int main(int argc, char *argv[]) {
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;
    int max_clients;

    //check if all required arguments
    if (argc < 3) {
        printf("./Server <port> <max clients>\n");
        return 0;
    }

    //initialize arrays with size defined on argument
    max_clients = atoi(argv[2]);
    threads = calloc(max_clients, sizeof(pthread_t));
    client_fds = calloc(max_clients, sizeof(int));

    //initialize server
    fim = 0;
    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("na funcao socket");
    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        erro("na funcao bind");
    if (listen(fd, 5) < 0)
        erro("na funcao listen");
    client_addr_size = sizeof(client_addr);

    //while no shutdown command is received keep waiting for clients
    while (!fim) {
        client = accept(fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_size);
        if (client > 0) {
            //find slot for client
            int slot = find_empty_slot(client_fds, max_clients);
            //check if slot was found, send error message and close connection if not
            if (slot == -1) {
                write(client, "riperino", sizeof("riperino"));
                close(client);
            } else { //otherwise create thread to handle client
                client_fds[slot] = client;
                pthread_create(threads + slot, NULL, process_client, client_fds + slot);
            }
        }
    }

    return 0;
}

void *process_client(void *arg) {
    int client_fd = *((int *) arg);
    int nread = 1;
    char buffer[BUF_SIZE];
    while(!fim && nread > 0){
        nread = read(client_fd, buffer, BUF_SIZE - 1);
        buffer[nread] = '\0';
        printf("%s\n", buffer);
    }
    close(client_fd);
    //free client slot
    *((int *) arg) = 0;
    pthread_exit(NULL);
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}

int find_empty_slot(int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (!array[i])
            return i;
    }
    return -1;
}