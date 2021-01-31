#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#include "command.h"
#include "ascii_read.h"

#include <pthread.h>

#define PORT 2121
#define BSIZE 256

int s;
const char *local_root = "root";

void cleanup(int _) {
    int r;
    fprintf(stderr, "Closing sockets\n");
    r = 0;
    if(s > 0) r=close(s);
    if (r == -1) perror("Close s");
    exit(0);
}

int set_nonblocking(int sockfd) {
    int reuse_addr_val = 1; 
    int r;
    r = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_val, sizeof(reuse_addr_val));
    if (r == -1)
        perror("Set reuse addr"), cleanup(0);
    return errno;
}

int server_say(int sockfd, int code, char *msg) {
    char buf[BSIZE];
    int n, r;
    n = snprintf(buf, BSIZE, "%d %s\r\n", code, msg);
    r = write(sockfd, buf, n);
    write(STDOUT_FILENO, buf, n);
    return r;
}

typedef long (*readf_t)(int, void*, long unsigned int, struct read_params *);

long reg_read(int fd, void* buf, long unsigned int n, struct read_params *unused) {
    return read(fd, buf, n);
}

struct transfer_params {
    int data_sock;
    readf_t read_f;
};
#ifndef TBSIZE
#define TBSIZE 3
#endif
int transfer(int c, int from_fd, int to_fd, readf_t read_f) {
    char buf[TBSIZE];
    int n;
    struct read_params params = {0, 0};
    if (from_fd == -1) {
        server_say(c, 550, "Could not open");
        return -1;
    } else {
        server_say(c, 150, "Transfer in progress");
        do {
            n = read_f(from_fd, buf, TBSIZE, &params);
            if (n == -1)
                perror("Read file");
            write(to_fd, buf, n);
        } while (n > 0);
        server_say(c, 226, "Transfer complete");
    }
    
    if (to_fd != c) {
        close(to_fd);
        to_fd = c;
    }
    return 0;
}

struct thread_params {
    int c;
};
enum state{INIT, IDLE, LOGIN, ERROR, FIN};
enum mode{ASCII, BINARY};

void *session_thread(void *arg) {
    struct thread_params *params = arg;
    int c = params->c;
    int r;
    int data_sock = c;
    char buf[BSIZE];
    
    char cwd[256] = {'/', 0};

    enum mode curr_mode = BINARY;
    enum state curr_state = INIT;

    struct command com;
    com.text = buf;

    while (curr_state != FIN) {

        switch (curr_state) {
            case INIT:
                server_say(c, 220, "Ready");
                curr_state = IDLE;
                break;
            case IDLE:
            {
                com_readn(&com, c, BSIZE);

                if (!com_cmp(&com, "USER")) {
                    printf("U\n");
                    curr_state = LOGIN;
                    server_say(c, 331, "password, please");
                    break;
                } 
                else if (!com_cmp(&com, "TYPE")) {
                    com_adv(&com);
                    if (!com_cmp(&com, "A")) {
                        curr_mode = ASCII;
                        server_say(c, 220, "Switch to ASCII");
                    } else if (!com_cmp(&com, "I")) {
                        curr_mode = BINARY;
                        server_say(c, 220, "Switch to BINARY");
                    } else {
                        server_say(c, 202, "Not implemented");
                    }
                } 
                else if (!com_cmp(&com, "LIST")) {
                    char *path = NULL;
                    struct dirent *dir;
                    path = com_get_path(&com, local_root, cwd);
                    DIR *d = opendir(path ? path : local_root);
                    if (path) {
                        printf(path);
                        free(path);
                    }
                    server_say(c, 150, "Sending directory listing");
                    while ((dir = readdir(d)) != NULL) {
                        if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")) {
                            write(data_sock, dir->d_name, strlen(dir->d_name));
                            if (dir->d_type == DT_DIR) {
                                write(data_sock, "/", 1);
                            }
                            write(data_sock, "\r\n", 2);
                        }
                    }
                    server_say(c, 226, "List sent");

                    close(data_sock);
                } 
                else if (!com_cmp(&com, "RETR")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    int fd;

                    fd = open(path, O_RDONLY);
                    free(path);
                    
                    transfer(c, fd, data_sock, curr_mode==BINARY ? reg_read : read_i2a);
                }
                else if (!com_cmp(&com, "STOR")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    int fd;

                    fd = open(path, O_WRONLY|O_CREAT, 0644);
                    free(path);
                    
                    transfer(c, data_sock, fd, curr_mode==BINARY ? reg_read : read_a2i);
                } 
                else if (!com_cmp(&com, "MKD")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    if (!path || (r = mkdir(path, 0755)) == -1) {
                        server_say(c, 550, "Operation failed");
                    } else {
                        server_say(c, 250, "Directory created");
                    }
                    if (path) free(path);
                } 
                else if (!com_cmp(&com, "RMD")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    if (!path || (r = rmdir(path)) == -1) {
                        server_say(c, 550, "Operation failed");
                    } else {
                        server_say(c, 250, "Directory removed");
                    }
                    if (path) free(path);
                } 
                else if (!com_cmp(&com, "SYST")) {
                    server_say(c, 215, "UNIX Type: L8");
                    break;
                } 
                else if (!com_cmp(&com, "PORT")) {
                    char arg[BSIZE] = {0};
                    com_adv(&com);
                    com_storen(&com, arg, BSIZE);

                    data_sock = socket(AF_INET, SOCK_STREAM, 0);

                    struct sockaddr_in peer_sa;
                    bzero(&peer_sa, sizeof(struct sockaddr_in));
                    int addr[6];
                    peer_sa.sin_family = AF_INET;
                    r = sscanf(arg, "%d,%d,%d,%d,%d,%d", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]);
                    peer_sa.sin_addr.s_addr = addr[0]+(1<<8)*addr[1]+(1<<16)*addr[2]+(1<<24)*addr[3];
                    peer_sa.sin_port = addr[4]+(1<<8)*addr[5];
                    printf("%s\n", arg);
                    
                    printf("Connecting to %s:", inet_ntoa(peer_sa.sin_addr));
                    printf("%d\n", ntohs(peer_sa.sin_port));

                    r = connect(data_sock, (struct sockaddr*)&peer_sa, sizeof(struct sockaddr_in));
                    if (r == -1) {
                        perror("Open data connection");
                        goto disconnect;
                    }
                    server_say(c, 200, "PORT command successful");
                } else if (!com_cmp(&com, "QUIT")) {
                    server_say(c, 221, "Goodbye");
                    curr_state = FIN;
                } else {
                    printf("N/I: %s\n", com.text);
                    server_say(c, 202, "Not implemented");
                }
                break;
            }
                
            case LOGIN:
            {
                com_readn(&com, c, BSIZE);

                if (!com_cmp(&com, "PASSWORD")) {
                    printf("P\n");
                    server_say(c, 230, "logged in");
                    curr_state = IDLE;
                }
                break;
            }
                
            default:
                break;
        }
        
    }
    
    disconnect:
    printf("Disconnecting client\n");
    close(c);
    c = -1;
    free(params);
    return NULL;
}
#define N_THREADS 16
int main() {
    struct sockaddr_in sin;
    int c;
    int r;

    printf("Transfer buffer size: %d\n", TBSIZE);
    
    signal(SIGINT, cleanup);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Create socket");
        return -1;
    }

    set_nonblocking(s);

    bzero(&sin, sizeof(struct sockaddr_in));
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    r = bind(s, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
    if (r != 0) {
        perror("Bind");
        goto fail;
    }

    r = listen(s, 5);

    printf("Listening on %s:%d...\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    int i = 0;
    pthread_t threads[N_THREADS] = {0};

    for (;;) {
        
        struct sockaddr_in cin;
        socklen_t slen = sizeof(struct sockaddr_in);
        c = accept(s, (struct sockaddr*)&cin, &slen);
        if (c == -1) {
            perror("Connect client");
            continue;
        }

        printf("Connection from %s:%d\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

        struct thread_params *params = malloc(sizeof(struct thread_params));
        params->c = c;
        pthread_create(&threads[i++], NULL, session_thread, params);
    }


    return 0;

    fail:
    close(s);
    return -1;
}