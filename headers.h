//
// Created by francisco on 14/12/2019.
//

#ifndef IRCPROJ_HEADERS_H
#define IRCPROJ_HEADERS_H

void *process_client(void *arg);

void erro(char *msg);

int find_empty_slot(int *array, int size);

void list_files(char *output);

#endif //IRCPROJ_HEADERS_H
