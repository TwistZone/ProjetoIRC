//
// Created by francisco on 14/12/2019.
//

#ifndef IRCPROJ_HEADERS_H
#define IRCPROJ_HEADERS_H

void *process_client(void *arg);

void erro(char *msg);

int find_empty_slot(const int *array, int size);

void list_files(char *output);

void to_lower(char *str);

void upload(FILE *fp, int client_fd);

#endif //IRCPROJ_HEADERS_H
