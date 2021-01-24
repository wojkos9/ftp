#include <unistd.h>

char a2i_cache = 0;
int cached_a2i = 0;
long read_a2i(int fd, void *dst, long unsigned int n) {
    int r, i;
    char buf;
    char *cdst = (char*)dst;
    buf = 0;
    i = 0;
    if (n < 1) return 0;
    if (cached_a2i) {
        
        if (a2i_cache == '\r') {
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
                    a2i_cache = buf;
                    cached_a2i = 1;
                    return i;
                }
            }
        } else {
            *cdst = a2i_cache;
        }
        i++;
        cached_a2i = 0;
    }

    for(; i < n; i++) {
        r = read(fd, &buf, 1);
        if (r <= 0) goto leave;
        
        if (buf == '\r') {
            a2i_cache = '\r';
            goto process_cache;
        } else {
            cdst[i] = buf;
        }
    }
    leave:
    return i ? i : r;
}

char i2a_cache = 0;
int cached_i2a = 0;
long read_i2a(int fd, void *dst, long unsigned int n) {
    int r, i;
    char *cdst = (char*)dst;
    i = 0;

    if (cached_i2a) {
        cached_i2a = 0;
        cdst[i++] = i2a_cache;
    }
    for(; i < n; i++) {
        r = read(fd, dst+i, 1);
        if (r <= 0) goto leave;
        
        if (cdst[i] == '\n') {
            cdst[i] = '\r';
            if (i == n-1) {
                i2a_cache = '\n';
                cached_i2a = 1;
            } else {
                cdst[++i] = '\n';
            }
            
        }
    }
    leave:
    return i ? i : r;
}