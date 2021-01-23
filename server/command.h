#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>
#include <unistd.h>

struct command {char *text; char *save; int toklen; int state;};

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
    return strncmp(com->save, cmps, com->toklen);
}

int com_storen(struct command *com, char *dst, int n) {
    if (n < com->toklen+1)
        return -1;
    strncpy(dst, com->save, com->toklen);
    return 0;
}

#endif