#ifndef COMMAND_H
#define COMMAND_H

#include "types.h"

// advance 1 word in command
int com_adv(struct command *com);

// read command into buffer[:n]
int com_readn(struct command *com, int sockfd, int n);

// case independent strcmp on current word in command
int com_cmp(struct command *com, char *cmps);

// store argument in dst[:n]
int com_storen(struct command *com, char *dst, int n);

// process next argument as path and return as "absolute" path
char* com_get_path(struct command *com, const char *local_root, char *cwd);

// create sockaddr_in from next command argument
int com_get_sockaddr(struct command *com, struct sockaddr_in *sa);

#endif