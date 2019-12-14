
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
#include <dirent.h>
#include <regex.h>
#include <ctype.h>
#include "headers.h"

#define BUF_SIZE 1024
#define DOWNLOAD_PATTERN "download (tcp|udp) (enc|nor) [a-zA-Z.]*"

pthread_t *threads;
int *client_fds;
char fim;

void *proper_termination_of_port(void *fd_pointer) {
    //for test purposes only, should not make it to final version.
    int fd = *((int *) fd_pointer);
    char buffer[BUF_SIZE];
    scanf("%s", buffer); //waits for anything
    close(fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;
    int max_clients;

    //check if all required arguments
    if (argc < 3) {
        printf("./Server <port> <max clients>\n");
        return 0;
    } else printf("server running on process %d\n", getpid());

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

    //for test purposes only, should not make it to final version.
    pthread_create(threads, NULL, proper_termination_of_port, &fd);

    //while no shutdown command is received keep waiting for clients
    while (!fim) {
        client = accept(fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_size);
        if (client > 0) {
            //find slot for client
            int slot = find_empty_slot(client_fds, max_clients);
            //check if slot was found, create thread to handle client or close connection if no slot avaiable
            if (slot != -1) {
                client_fds[slot] = client;
                pthread_create(threads + slot, NULL, process_client, client_fds + slot);
            } else {
                close(client);
            }
        }
    }
    close(fd);
    return 0;
}

void *process_client(void *arg) {
    int client_fd = *((int *) arg);
    int nread = 1;
    char buffer[BUF_SIZE], encryption[4], protocol[4], file_name[BUF_SIZE], file_path[BUF_SIZE];
    regex_t regex;
    FILE *fp;

    //prepare regex to confirm correctness of download requests
    regcomp(&regex, DOWNLOAD_PATTERN, REG_EXTENDED);

    while (!fim && nread > 0) {
        nread = read(client_fd, buffer, BUF_SIZE - 1);
        buffer[nread] = '\0';
        printf("%s\n", buffer);
        to_lower(buffer);
        if (!strcmp(buffer, "list")) {
            list_files(buffer);
        } else if (!strcmp(buffer, "quit")) {
            break;
        } else if (!regexec(&regex, buffer, 0, NULL, 0)) {
            //TODO process options enc/protocol
            //find file, if does not exist send error otherwise send file size and then send file
            sscanf(buffer, "download %s %s %s", protocol, encryption, file_name);
            sprintf(file_path, "server_files/%s", file_name);
            fp = fopen(file_path, "rb");
            if (fp == NULL) {
                strcpy(buffer, "requested file not available");
            } else {
                upload(fp, client_fd, file_name);
            }

        } else {
            strcpy(buffer, "unknown command");
        }
        write(client_fd, buffer, strlen(buffer) + 1);
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

int find_empty_slot(const int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (!array[i])
            return i;
    }
    return -1;
}

void list_files(char *output) {
    //based on source code from web
    //https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
    DIR *dr = opendir("server_files");
    struct dirent *de;
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        strcpy(output, "Could not open the directory");
        return;
    }
    *output = 0; //initialize string to concatenate over
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            strcat(output, de->d_name);
            strcat(output, "\n");
        }
    }

    closedir(dr);
}

void to_lower(char *str) {
    //source code from web
    //https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c
    for (int i = 0; str[i]; i++) {
        str[i] = (char) tolower(str[i]);
    }
}

void upload(FILE *fp, int client_fd, char *file_name) {
    char buffer[BUF_SIZE];
    unsigned long n_read;
    fseek(fp, 0L, SEEK_END);
    sprintf(buffer, "download file %s with %ld bytes", file_name, ftell(fp));
    rewind(fp);
    write(client_fd, buffer, strlen(buffer) + 1);
    //send file
    while ((n_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(client_fd, buffer, n_read, 0);
    }
    fclose(fp);
}