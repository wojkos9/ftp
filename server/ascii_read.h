#ifndef ASCII_READ_H
#define ASCII_READ_H
#include <unistd.h>

#include "utils.h"

// read from ASCII to binary
long read_a2i(int fd, void *dst, long unsigned int n, struct read_params *params) {
    char *cache = &params->cache;
    int *is_cached = &params->is_cached;
    int r, i;
    char buf;
    char *cdst = (char*)dst;
    buf = 0;
    i = 0;
    if (n < 1) return 0;
    if (*is_cached) {
        
        if (*cache == '\r') {
            process_cache:
            r = read(fd, &buf, 1);
            if (r <= 0) goto leave;

            // if sequence was \r\n, store just the \n
            if (buf == '\n') {
                cdst[i] = buf;
            } 
            // otherwise, keep processing
            else {
                cdst[i] = '\r';
                i++;
                if (i < n) {
                    if (buf == '\r')
                        goto process_cache;
                    cdst[i] = buf;
                } 
                // if buffer is to overflow -> cache
                else {
                    *cache = buf;
                    *is_cached = 1;
                    return i;
                }
            }
        } else {
            *cdst = *cache;
        }
        i++;
        *is_cached = 0;
    }
    // read char by char until <CR> occurs, then process it
    for(; i < n; i++) {
        r = read(fd, &buf, 1);
        if (r <= 0) goto leave;
        
        if (buf == '\r') {
            *cache = '\r';
            goto process_cache;
        } else {
            cdst[i] = buf;
        }
    }
    leave:
    return i ? i : r;
}

// read from binary to ASCII
long read_i2a(int fd, void *dst, long unsigned int n, struct read_params *params) {
    int r, i;
    char *cdst = (char*)dst;
    char *cache = &params->cache;
    int *is_cached = &params->is_cached;
    
    i = 0;
    if (n < 1) return 0;
    // write what was cached (<LF>)
    if (*is_cached) {
        *is_cached = 0;
        cdst[i++] = *cache;
    }
    for(; i < n; i++) {
        r = read(fd, dst+i, 1);
        if (r <= 0) goto leave;
        
        if (cdst[i] == '\n') {
            cdst[i] = '\r';
            
            // if no space in buffer, store <LF> for later
            if (i == n-1) {
                *cache = '\n';
                *is_cached = 1;
            } else {
                cdst[++i] = '\n';
            }
            
        }
    }
    leave:
    return i ? i : r;
}
#endif