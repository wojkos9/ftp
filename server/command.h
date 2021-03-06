#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>
#include <unistd.h>

#include "utils.h"

struct command {char *text; char *save; int toklen; int state;};

// advance 1 word in command
int com_adv(struct command *com) {
    if (0 != com->state) {
        return com->state;
    }
    if (!com->save) {
        com->save = com->text;
    } else {
        com->save += com->toklen+1;
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

// read command into buffer[:n]
int com_readn(struct command *com, int sockfd, int n) {
    int r;
    com->state = 0;
    com->save = NULL;
    com->toklen = 0;
    int i = 0;
    do {
        r = read(sockfd, com->text+i, 1);
        if (r == -1) {
            return -1;
        }
        i++;
        if (com->text[i-1] == '\n')
            break;
    } while (i < n-1 && r > 0);
    com->text[i]=0;
    if (!com_adv(com))
        return -1;
    return i;
}

// case independent strcmp on current word in command
int com_cmp(struct command *com, char *cmps) {
    int r, i, n;
    i = 0;
    n = com->toklen;
    while (i < n && !(r=(com->save[i]&0xdf)-cmps[i])) i++;
    return r;
}

// store argument in dst[:n]
int com_storen(struct command *com, char *dst, int n) {
    if (n < com->toklen+1)
        return -1;
    strncpy(dst, com->save, com->toklen);
    return 0;
}
#define BSIZE1 256
// process next argument as path and return as "absolute" path
char* com_get_path(struct command *com, const char *local_root, char *cwd) {
    char arg[BSIZE1] = {0};
    char *buf;
    int abs;
    int r;
    r = com_adv(com);
    if (r) {
        return NULL;
    }
    com_storen(com, arg, BSIZE1);
    
    buf = (char*) malloc(BSIZE1);
    abs = arg[0] == '/';
    snprintf(buf, BSIZE1, abs ? "%s%s" : "%s%s%s", local_root, abs ? arg : cwd, arg);
    return buf;
}

// create sockaddr_in from next command argument
int com_get_sockaddr(struct command *com, struct sockaddr_in *sa) {
    int r;
    char arg[BSIZE1] = {0};
    com_adv(com);
    com_storen(com, arg, BSIZE1);

    r = sockaddr_from_str(arg, sa);
    return r != 6;
}

#endif