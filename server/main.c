#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include "types.h"
#include "consts.h"
#include "utils.h"
#include "session.h"


// main server socket descriptor
int s;

// SIGINT handler
void cleanup(int _) {
    int r;
    fprintf(stderr, "\nClosing server socket\n");
    r = 0;
    if(s > 0) r=close(s);
    if (r == -1) perror("Close s");
    exit(0);
}


int main(int argc, char *argv[]) {
    struct sockaddr_in sin;
    int c;
    int r;
    int port = PORT;
    char local_root[64] = {0};


    // process command line arguments
    char opt;
    while ((opt = getopt(argc, argv, ":p:r:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                if (port <= 0) {
                    fprintf(stderr, "Incorrect port number\n");
                    return -1;
                }
                break;
            case 'r':
                if (dir_exists(optarg)) {
                    strncpy(local_root, optarg, 64);
                } else {
                    fprintf(stderr, "Directory %s does not exists. Using default: %s\n", optarg, DEFAULT_ROOT);
                }
                break;
        }
    }
    if (!*local_root)
        strcpy(local_root, DEFAULT_ROOT);

    printf("Transfer buffer size: %d\n", TBSIZE);
    printf("Root directory: %s\n", local_root);

    signal(SIGINT, cleanup);


    // prepare server socket
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Create socket");
        return -1;
    }

    set_nonblocking(s);

    // bind to 0.0.0.0:port
    bzero(&sin, sizeof(struct sockaddr_in));
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
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

    // accept clients in a loop
    for (;;) {
        struct sockaddr_in cin;
        socklen_t slen = sizeof(struct sockaddr_in);
        c = accept(s, (struct sockaddr*)&cin, &slen);
        if (c == -1) {
            perror("Connect client");
            continue;
        }

        printf("Connection from %s:%d\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

        // create session thread for accepted connection
        struct thread_params *params = malloc(sizeof(struct thread_params));
        params->local_root = local_root;
        params->c = c;
        pthread_create(&threads[i++], NULL, session_thread, params);
    }


    return 0;

    fail:
    close(s);
    return -1;
}