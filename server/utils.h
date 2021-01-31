
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

int set_nonblocking(int sockfd) {
    int reuse_addr_val = 1; 
    int r;
    r = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_val, sizeof(reuse_addr_val));
    if (r == -1)
        perror("Set reuse addr");
    return errno;
}

int server_say(int sockfd, int code, char *msg) {
    char buf[256];
    int n, r;
    n = snprintf(buf, 256, "%d %s\r\n", code, msg);
    r = write(sockfd, buf, n);
    write(STDOUT_FILENO, buf, n);
    return r;
}

struct read_params {
    char cache;
    int is_cached;
};

#endif