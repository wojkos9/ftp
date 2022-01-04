#ifndef TRANSFER_H
#define TRANSFER_H

#include <pthread.h>
#include "types.h"


// wrapper for compatibility
long simple_read(int fd, void* buf, long unsigned int n, struct read_params *unused);

// sends file over data connection
void *transfer_thread(void *args);

// wrapper for creating the transfer thread
pthread_t transfer(int c, int from_fd, int to_fd, readf_t read_f, int *transfer_fin);

#endif
