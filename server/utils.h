#ifndef UTILS_H
#define UTILS_H

#include <netinet/in.h>

// set SO_REUSEADDR on socket
int set_nonblocking(int sockfd);

// send message to client in standard format
int server_say(int sockfd, int code, char *msg);

int sockaddr_from_str(char *arg, struct sockaddr_in *sa);

int dir_exists(const char *path);

#endif