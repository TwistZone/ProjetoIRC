#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <regex.h>

#define BUF_SIZE 1000
#define DOWNLOAD_PATTERN "download file [a-zA-Z.]* with [0-9]* bytes"

void erro(char *msg);

void download(int fd, char *buffer);

int main(int argc, char *argv[]) {
    char endServer[BUF_SIZE];
    char buffer[BUF_SIZE];
    int fd;
    int fim = 0, n_read;
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
    addr.sin_addr.s_addr = ((struct in_addr *) (hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[4]));

    //sets up socket and tries connection
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        erro("socket");
    if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        erro("Connect");
    while (!fim) {
        //get user input and send to server
        fgets(buffer, BUF_SIZE, stdin);
        buffer[strlen(buffer) - 1] = 0; //remove \n
        //in case of exit end cycle
        if (!strcasecmp(buffer, "quit")) {
            fim = 1;
        }
        write(fd, buffer, strlen(buffer));
        n_read = read(fd, buffer, BUF_SIZE);
        //verify if download
        download(fd, buffer);
        printf("%s\n", buffer);
    }
    close(fd);
    exit(0);
}

void download(int fd, char *buffer) {
    char file_name[BUF_SIZE];
    int total, n_read;
    FILE *fp;
    regex_t regex;
    regcomp(&regex, DOWNLOAD_PATTERN, REG_EXTENDED);
    //if download message, downloads, else does nothing and prints message on main as usual
    if (!regexec(&regex, buffer, 0, NULL, 0)) {
        sscanf(buffer, "download file %s with %d bytes", file_name, &total);
        fp = fopen(file_name, "wb");
        while (total > 0 && (n_read = recv(fd, buffer, 1024, 0)) > 0) {
            total -= n_read;
            fwrite(buffer, 1, n_read, fp);
        }
        fclose(fp);
        strcpy(buffer, "download success");
    }
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
