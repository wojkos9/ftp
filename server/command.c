#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "types.h"
#include "consts.h"

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


int com_cmp(struct command *com, char *cmps) {
    int r, i, n;
    i = 0;
    n = com->toklen;
    while (i < n && !(r=(com->save[i]&0xdf)-cmps[i])) i++;
    return r;
}


int com_storen(struct command *com, char *dst, int n) {
    if (n < com->toklen+1)
        return -1;
    strncpy(dst, com->save, com->toklen);
    return 0;
}


char* com_get_path(struct command *com, const char *local_root, char *cwd) {
    char arg[MAX_ARGSIZE] = {0};
    char *buf;
    int abs;
    int r;
    r = com_adv(com);
    if (r) {
        return NULL;
    }
    com_storen(com, arg, MAX_ARGSIZE);

    buf = (char*) malloc(MAX_ARGSIZE);
    abs = arg[0] == '/';
    snprintf(buf, MAX_ARGSIZE, abs ? "%s%s" : "%s%s%s", local_root, abs ? arg : cwd, arg);
    return buf;
}


int com_get_sockaddr(struct command *com, struct sockaddr_in *sa) {
    int r;
    char arg[MAX_ARGSIZE] = {0};
    com_adv(com);
    com_storen(com, arg, MAX_ARGSIZE);

    r = sockaddr_from_str(arg, sa);
    return r != 6;
}
