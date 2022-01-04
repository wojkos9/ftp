#ifndef ASCII_READ_H
#define ASCII_READ_H

// read from ASCII to binary
long read_a2i(int fd, void *dst, long unsigned int n, struct read_params *params);

// read from binary to ASCII
long read_i2a(int fd, void *dst, long unsigned int n, struct read_params *params);

#endif