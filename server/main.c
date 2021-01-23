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

#include "command.h"

#include <pthread.h>

#define PORT 2121
#define BSIZE 256

int s;
int r;
int c;

void cleanup(int _) {
    fprintf(stderr, "Closing sockets\n");
    r = 0;
    if(c > 0) r=close(c);
    if (r == -1) perror("Close c");
    if(s > 0) r=close(s);
    if (r == -1) perror("Close s");
    exit(0);
}

int set_nonblocking(int sockfd) {
    int reuse_addr_val = 1; 
    r = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_val, sizeof(reuse_addr_val));
    if (r == -1)
        perror("Set reuse addr"), cleanup(0);
    return errno;
}

int server_say(int sockfd, int code, char *msg) {
    char buf[BSIZE];
    r = snprintf(buf, BSIZE, "%d %s\r\n", code, msg);
    r = write(sockfd, buf, r);
    return 0;
}

int main() {
    struct sockaddr_in sin;
    const char *local_root = "root";
    char cwd[256] = {'/', 0};

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

    for (;;) {
        int data_sock = c;
        char buf[BSIZE];
        struct sockaddr_in cin;
        socklen_t slen = sizeof(struct sockaddr_in);
        c = accept(s, (struct sockaddr*)&cin, &slen);
        if (c == -1) {
            perror("Connect client");
            goto fail;
        }

        printf("Connection from %s:%d\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

        enum state{INIT, IDLE, LOGIN, ERROR, FIN};
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
                    } else if (!com_cmp(&com, "LIST")) {
                        char arg[BSIZE] = {0};
                        r = com_adv(&com);
                        if (!r) {
                            com_storen(&com, arg, BSIZE);
                            printf("lsarg %s\n", arg);
                        } else {
                            printf("ls\n");
                        }
                        server_say(c, 150, "Sending directory listing");
                        write(data_sock, "file\r\n", 6);
                        server_say(c, 226, "List sent");
                        if (data_sock != c) {
                            close(data_sock);
                        }
                    } else if (!com_cmp(&com, "RETR")) {
                        char arg[BSIZE] = {0};
                        char buf[BSIZE] = {0};
                        int fd = -1;
                        char *t, *e;
                        e = buf+BSIZE;
                        int n;
                        com_adv(&com);
                        com_storen(&com, arg, BSIZE);
                        
                        int abs = arg[0] == '/';
                        snprintf(buf, BSIZE, abs ? "%s%s" : "%s%s%s", local_root, abs ? arg : cwd, arg);

                        fd = open(buf, O_RDONLY);
                        if (fd == -1) {
                            server_say(c, 550, "Could not open");
                        } else {
                            server_say(c, 150, "Binary transfer");
                            do {
                                n = read(fd, buf, BSIZE);
                                if (n == -1)
                                    perror("Read file");
                                r = write(data_sock, buf, n);
                            } while (n > 0);
                            server_say(c, 226, "Transfer complete");
                        }
                        
                        if (data_sock != c) {
                            close(data_sock);
                        }
                    } 
                    
                    else if (!com_cmp(&com, "SYST")) {
                        server_say(c, 215, "Linux");
                        break;
                    } else if (!com_cmp(&com, "PORT")) {
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
                        break;
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
        close(c);
    }


    return 0;

    fail:
    close(s);
    return -1;
}