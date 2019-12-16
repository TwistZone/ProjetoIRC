//
// Created by francisco on 14/12/2019.
//

#ifndef IRCPROJ_SERVER_HEADERS_H
#define IRCPROJ_SERVER_HEADERS_H

void *process_client(void *arg);

void erro(char *msg);

int find_empty_slot(const int *array, int size);

void list_files(char *output);

void to_lower(char *str);

void upload_tcp(FILE *fp, int client_fd, char *file_name, int encryption);

void upload_udp(FILE *fp, int client_fd, char *file_name, int encryption);

#endif //IRCPROJ_SERVER_HEADERS_H
