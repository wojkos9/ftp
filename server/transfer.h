#ifndef TRANSFER_H
#define TRANSFER_H

#include "utils.h"

#include <stdlib.h>
#include <pthread.h>

struct read_params {
    char cache;
    int is_cached;
};

typedef long (*readf_t)(int, void*, long unsigned int, struct read_params *);

// wrapper for compatibility
long reg_read(int fd, void* buf, long unsigned int n, struct read_params *unused) {
    return read(fd, buf, n);
}

struct transfer_params {
    int c; int from_fd; int to_fd; readf_t read_f; int *transfer_fin;
};

#ifndef TBSIZE
#define TBSIZE 4096
#endif

// sends file over data connection
void *transfer_thread(void *args) {
    struct transfer_params *params = args;
    int c = params->c;
    int from_fd = params->from_fd;
    int to_fd = params->to_fd;
    readf_t read_f = params->read_f;
    char buf[TBSIZE];
    int n;
    int r;
    struct read_params r_params = {0, 0};

    if (from_fd < 0 || to_fd < 0) {
        server_say(c, 550, "Could not open");
        *(params->transfer_fin) = 2;
    } else {
        server_say(c, 150, "Transfer in progress");
        do {
            n = read_f(from_fd, buf, TBSIZE, &r_params);
            if (n == -1) {
                perror("Transfer from fd");
            }
                
            r = write(to_fd, buf, n);
            if (r < 0) {
                perror("Transfer to fd");

            }
        } while (n > 0 && r > 0);
        if (!(n < 0 || r < 0))
            server_say(c, 226, "Transfer complete");
        else
            server_say(c, 451, "An error ocurred during transfer");

        close(from_fd);
        close(to_fd);
    }
    
    
    *(params->transfer_fin) = 1;

    free(params);
    return NULL;
}

// wrapper for creating the transfer thread
pthread_t transfer(int c, int from_fd, int to_fd, readf_t read_f, int *transfer_fin) {
    struct transfer_params *params = malloc(sizeof(struct transfer_params));
    params->c = c;
    params->from_fd = from_fd;
    params->to_fd = to_fd;
    params->read_f = read_f;
    params->transfer_fin = transfer_fin;
    *transfer_fin = 0;
    pthread_t t;
    pthread_create(&t, NULL, transfer_thread, params);
    return t;
}
#endif