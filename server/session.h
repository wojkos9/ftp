#ifndef SESSION_H
#define SESSION_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include <stdlib.h>

#include "transfer.h"
#include "ascii_read.h"
#include "utils.h"
#include "command.h"
#include <pthread.h>

#define BSIZE 256

struct thread_params {
    int c;
    const char *local_root;
};
enum state{INIT, IDLE, LOGIN, FIN};
enum mode{ASCII, BINARY};


void *session_thread(void *args) {
    struct thread_params *params = args;
    const char *local_root = params->local_root;
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

    // state machine
    while (curr_state != FIN) {

        switch (curr_state) {
            case INIT:
                server_say(c, 220, "Ready");
                curr_state = IDLE;
                break;
            case IDLE:
            {
                // read next command
                com_readn(&com, c, BSIZE);

                // compare first word of command
                if (!com_cmp(&com, "USER")) {
                    curr_state = LOGIN; // if logging in, wait for password
                    server_say(c, 331, "password, please");
                    break;
                }
                // ascii / binary
                else if (!com_cmp(&com, "TYPE")) {
                    // go to the next word
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
                // ls [arg]
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
                    closedir(d);
                    server_say(c, 226, "List sent");

                    close(data_sock);
                }
                // get
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
                // put
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
                // mkdir
                else if (!com_cmp(&com, "MKD")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    if (!path || (r = mkdir(path, 0755)) == -1) {
                        server_say(c, 550, "Creating directory failed");
                    } else {
                        server_say(c, 257, "Directory created");
                    }
                    if (path) free(path);
                }
                // rmdir
                else if (!com_cmp(&com, "RMD")) {
                    char *path = com_get_path(&com, local_root, cwd);
                    if (!path || (r = rmdir(path)) == -1) {
                        server_say(c, 550, "Removing directory failed");
                    } else {
                        server_say(c, 250, "Directory removed");
                    }
                    if (path) free(path);
                }
                // send system info (for compatibility with existing clients)
                else if (!com_cmp(&com, "SYST")) {
                    server_say(c, 215, "UNIX Type: L8");
                    break;
                }
                // for creating a new data connection
                else if (!com_cmp(&com, "PORT")) {
                    struct sockaddr_in peer_sa;

                    // on PORT a,b,c,d,e,f -> connect to a.b.c.d:ef (data connection)
                    data_sock = socket(AF_INET, SOCK_STREAM, 0);
                    r = com_get_sockaddr(&com, &peer_sa);
                    if (r) {
                        server_say(c, 501, "Syntax error");
                        break;
                    }
                    
                    printf("Connecting to %s:", inet_ntoa(peer_sa.sin_addr));
                    printf("%d\n", ntohs(peer_sa.sin_port));

                    r = connect(data_sock, (struct sockaddr*)&peer_sa, sizeof(struct sockaddr_in));
                    if (r == -1) { // client did not listen on the port it sent
                        server_say(c, 421, "Data connection cannot be established");
                    } else {
                        server_say(c, 200, "PORT command successful");
                    }
                }
                // abort
                else if (!com_cmp(&com, "ABOR")) {
                    // if file transfer in progress, stop the thread
                    if (!transfer_fin) {
                        r=pthread_cancel(transfer_th);
                        if (r != 0) {
                            perror("Cancel thread");
                            server_say(c, 421, "Abort error");
                        } else {
                            close(data_sock);
                            server_say(c, 226, "Abort successful");
                        }
                    } else {
                        server_say(c, 225, "Nothing to abort");
                    }
                    curr_state = IDLE;

                }
                // quit
                else if (!com_cmp(&com, "QUIT")) {
                    server_say(c, 221, "Goodbye");
                    curr_state = FIN;
                } 
                // if command code not recognized, inform client
                else {
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
                } else if (!com_cmp(&com, "ABOR")) {
                    server_say(c, 226, "Abort successful");
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

#endif