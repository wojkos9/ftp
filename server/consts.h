#ifndef CONSTS_H
#define CONSTS_H

// server port
#define PORT 2121

// path from which the server serves files by default
#define DEFAULT_ROOT "root"

// maximum number of client connections
#define N_THREADS 16

// size of the buffer user for transfer
#define TBSIZE 4096

// max length for filesystem paths
#define MAX_PATHLEN 256

// limit to which FTP commands will be truncated
#define MAX_CMDLEN 256

// limit to which arguments in FTP commands will be truncated
#define MAX_ARGSIZE 256

#endif
