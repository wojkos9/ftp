
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <netinet/in.h>
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

int sockaddr_from_str(char *arg, struct sockaddr_in *sa) {
    int r;
    bzero(sa, sizeof(struct sockaddr_in));
    int addr[6];
    sa->sin_family = AF_INET;
    r = sscanf(arg, "%d,%d,%d,%d,%d,%d", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]);
    sa->sin_addr.s_addr = addr[0]+(1<<8)*addr[1]+(1<<16)*addr[2]+(1<<24)*addr[3];
    sa->sin_port = addr[4]+(1<<8)*addr[5];
    return r;
}

int dir_exists(const char *path) {
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else if (errno == ENOENT) {
        return 0;
    } else {
        return -1;
    }
}

#endif