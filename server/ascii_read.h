#ifndef ASCII_READ_H
#define ASCII_READ_H
#include <unistd.h>

#include "utils.h"


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

            if (buf == '\n') {
                cdst[i] = buf;
            } else {
                cdst[i] = '\r';
                i++;
                if (buf == '\r')
                    goto process_cache;
                if (i < n) {
                    cdst[i] = buf;
                } else {
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

long read_i2a(int fd, void *dst, long unsigned int n, struct read_params *params) {
    int r, i;
    char *cdst = (char*)dst;
    char *cache = &params->cache;
    int *is_cached = &params->is_cached;
    
    i = 0;

    if (*is_cached) {
        *is_cached = 0;
        cdst[i++] = *cache;
    }
    for(; i < n; i++) {
        r = read(fd, dst+i, 1);
        usleep(10000);
        if (r <= 0) goto leave;
        
        if (cdst[i] == '\n') {
            cdst[i] = '\r';
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