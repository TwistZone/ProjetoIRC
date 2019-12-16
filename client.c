#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <regex.h>
#include <sodium.h>

#define BUF_SIZE 1024
#define DOWNLOAD_PATTERN "download (encrypted|clear) file [a-zA-Z.]* with [0-9]* bytes"

void erro(char *msg);

void download_tcp(int fd, char *buffer);

void download_udp(int fd, char *buffer);

int main(int argc, char *argv[]) {
    char endServer[BUF_SIZE];
    char buffer[BUF_SIZE];
    int fd;
    char fim = 0;
    int n_read;
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
            write(fd, buffer, strlen(buffer));
            fim = 1;
            sleep(1);
            exit(0);
        }
        write(fd, buffer, strlen(buffer));
        if (strstr(buffer, "download tcp")) {
            n_read = read(fd, buffer, BUF_SIZE);
            //verify if download
            download_tcp(fd, buffer);
        } else if (strstr(buffer, "download udp")) {
            n_read = read(fd, buffer, BUF_SIZE);
            //verify if download
            download_udp(fd, buffer);
        } else n_read = read(fd, buffer, BUF_SIZE);
        printf("%s\n", buffer);
    }
    close(fd);
    exit(0);
}

void download_udp(int fd, char *buffer) {
    printf("placeholder: %s\n", buffer);
}

void download_tcp(int fd, char *buffer) {
    char file_name[BUF_SIZE];
    unsigned char key[crypto_secretbox_KEYBYTES];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char encrypted[BUF_SIZE];
    unsigned char string[BUF_SIZE];
    int total, n_read;
    FILE *fp;
    regex_t regex;
    regcomp(&regex, DOWNLOAD_PATTERN, REG_EXTENDED);
    //if download message, downloads, else does nothing and prints message on main as usual
    if (!regexec(&regex, buffer, 0, NULL, 0)) {
        if (strstr(buffer, "encrypted")) {
            sscanf(buffer, "download encrypted file %s with %d bytes", file_name, &total);
            //get nonce
            recv(fd, nonce, crypto_secretbox_NONCEBYTES, 0);
            //load PSK
            fp = fopen("PSK", "rb");
            fread(key, 1, crypto_secretbox_KEYBYTES, fp);
            fclose(fp);
        } else {
            sscanf(buffer, "download clear file %s with %d bytes", file_name, &total);
        }
        fp = fopen(file_name, "wb");
        int extra = strstr(buffer, "encrypted") != NULL ? crypto_secretbox_MACBYTES : 0;
        while (total > 0 && (n_read = recv(fd, encrypted, BUF_SIZE < total + extra ? BUF_SIZE : total + extra, 0)) > 0) {
            total -= n_read - extra;
            if (strstr(buffer, "encrypted")) {
                int erro = crypto_secretbox_open_easy(string, encrypted, n_read, nonce, key);
                if (erro) {
                    printf("erro\n");
                    fclose(fp);
                    return;
                }
                fwrite(string, 1, n_read - crypto_secretbox_MACBYTES, fp);
            } else {
                fwrite(encrypted, 1, n_read, fp);
            }
        }
        fclose(fp);
        printf("download success\n");
        n_read = read(fd, buffer, BUF_SIZE);//read rest of input
    }
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}
