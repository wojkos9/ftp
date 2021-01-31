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
#include "utils.h"
#include "transfer.h"
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


struct thread_params {
    int c;
};
enum state{INIT, IDLE, LOGIN, FIN};
enum mode{ASCII, BINARY};

void *session_thread(void *arg) {
    struct thread_params *params = arg;
    int c = params->c;
    int r;
    int data_sock = c;
    char buf[BSIZE];
    int transfer_fin = 1;
    
    char cwd[256] = {'/', 0};

    enum mode curr_mode = BINARY;
    enum state curr_state = INIT;

    struct command com;
    com.text = buf;

    pthread_t transfer_th = 0;

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
                        server_say(c, 502, "Not implemented");
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
                    char *path;
                    int fd;

                    if (transfer_fin) {
                        path = com_get_path(&com, local_root, cwd);
                        fd = open(path, O_RDONLY);
                        free(path);
                        if (fd == -1) {
                            server_say(c, 550, "Could not read file");
                        } else {
                            transfer_th = transfer(c, fd, data_sock, curr_mode==BINARY ? reg_read : read_i2a, &transfer_fin);
                        }
                    } else {
                        server_say(c, 450, "Another transfer in progress");
                    }
                }
                else if (!com_cmp(&com, "STOR")) {
                    char *path;
                    int fd;

                    if (transfer_fin) {
                        path = com_get_path(&com, local_root, cwd);
                        fd = open(path, O_WRONLY|O_CREAT, 0644);
                        free(path);
                        if (fd == -1) {
                            server_say(c, 550, "Could not create file");
                        } else {
                            transfer_th = transfer(c, data_sock, fd, curr_mode==BINARY ? reg_read : read_a2i, &transfer_fin);
                        }
                    } else {
                        server_say(c, 450, "Another transfer in progress");
                    }
                } 
                else if (!com_cmp(&com, "MKD")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    if (!path || (r = mkdir(path, 0755)) == -1) {
                        server_say(c, 550, "Creating directory failed");
                    } else {
                        server_say(c, 257, "Directory created");
                    }
                    if (path) free(path);
                } 
                else if (!com_cmp(&com, "RMD")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    if (!path || (r = rmdir(path)) == -1) {
                        server_say(c, 550, "Removing directory failed");
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
                    printf("r: %d\n", r);
                    if (r != 6) {
                        server_say(c, 501, "Syntax error");
                        break;
                    }
                    peer_sa.sin_addr.s_addr = addr[0]+(1<<8)*addr[1]+(1<<16)*addr[2]+(1<<24)*addr[3];
                    peer_sa.sin_port = addr[4]+(1<<8)*addr[5];
                    
                    printf("Connecting to %s:", inet_ntoa(peer_sa.sin_addr));
                    printf("%d\n", ntohs(peer_sa.sin_port));

                    r = connect(data_sock, (struct sockaddr*)&peer_sa, sizeof(struct sockaddr_in));
                    if (r == -1) {
                        perror("Open data connection");
                        server_say(c, 421, "Data connection cannot be established");
                    } else {
                        server_say(c, 200, "PORT command successful");
                    }
                    
                } else if (!com_cmp(&com, "ABOR")) {
                    if (!transfer_fin) {
                        r=pthread_cancel(transfer_th);
                        if (r != 0) {
                            perror("Cancel thread");
                            server_say(c, 421, "Abort error");
                        } else {
                            close(data_sock);
                            server_say(c, 226, "Abort successful");
                        }
                    } else if (curr_state == LOGIN){
                        server_say(c, 226, "Abort successful");
                    } else {
                        server_say(c, 225, "Nothing to abort");
                    }
                    
                    curr_state = IDLE;
                } else if (!com_cmp(&com, "QUIT")) {
                    server_say(c, 221, "Goodbye");
                    curr_state = FIN;
                } else {
                    printf("N/I: %s\n", com.text);
                    server_say(c, 502, "Not implemented");
                }
                break;
            }
                
            case LOGIN:
            {
                com_readn(&com, c, BSIZE);

                if (!com_cmp(&com, "PASS")) {
                    server_say(c, 230, "logged in");
                    curr_state = IDLE;
                }
                break;
            }
                
            default:
                break;
        }
        
    }
    
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