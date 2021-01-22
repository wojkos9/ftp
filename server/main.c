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

struct command {char *text; char *save; int toklen; int state;};

int com_adv(struct command *com) {
    if (0 != com->state) {
        return com->state;
    }
    if (!com->save) {
        com->save = com->text;
    } else {
        com->save += com->toklen;
    }
    char *next = strchr(com->save, ' ');
    if (!next) {
        next = strchr(com->save, '\r');
        if (!next) {
            com->state = -1;
            return -1;
        } else {
            com->state = 1;
        }
    }
        
    
        
    com->toklen = next - com->save;

    return 0;
}

int com_readn(struct command *com, int sockfd, int n) {
    com->state = 0;
    com->save = NULL;
    com->toklen = 0;
    int i = 0;
    do {
        r = read(sockfd, com->text+i, 1);
        if (n == -1) {
            return -1;
        }
        if (com->text[i] == '\n')
            break;
        i++;
    } while (i < n && r > 0);
    if (!com_adv(com))
        return -1;
    return i;
}


int com_advn(struct command *com, int n) {
    while (n--) {
        r = com_adv(com);
    }
    return r;
}

int com_cmp(struct command *com, char *cmps) {
    return strncmp(com->save, cmps, com->toklen);
}

int com_storen(struct command *com, char *dst, int n) {
    if (n < com->toklen+1)
        return -1;
    strncpy(dst, com->save, com->toklen);
    return 0;
}

int server_say(int sockfd, int code, char *msg) {
    char buf[BSIZE];
    r = snprintf(buf, BSIZE, "%d %s\r\n", code, msg);
    r = write(sockfd, buf, r);
    return 0;
}

int main() {
    struct sockaddr_in sin;

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
                        write(c, "file\r\n", 6);
                        server_say(c, 226, "List sent");
                        curr_state = FIN;
                    } else if (!com_cmp(&com, "SYST")) {
                        server_say(c, 215, "Linux");
                        break;
                    } else if (!com_cmp(&com, "PORT")) {
                        // TODO
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
        

        close(c);
        break;
    }


    return 0;

    fail:
    close(s);
    return -1;
}