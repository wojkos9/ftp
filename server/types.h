#ifndef TYPES_H
#define TYPES_H

struct command {
    char *text;
    char *save;
    int toklen;
    int state;
};

struct read_params {
    char cache;
    int is_cached;
};

typedef long (*readf_t)(int, void*, long unsigned int, struct read_params *);

struct transfer_params {
    int c;
    int from_fd;
    int to_fd;
    readf_t read_f;
    int *transfer_fin;
};

struct thread_params {
    int c;
    const char *local_root;
};

#endif
