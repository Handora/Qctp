// all details in pool.c
#ifndef POOL_H
#define POOL_H
#include <stdio.h>
#include <sys/types.h>
#include <openssl/ssl.h>
#include "qio.h"
#include "parse.h"
#include <sys/stat.h>

// the max size of Descriptor size
#define FD_SIZE 1024
#define MAX_BODYSIZE 8192

enum State { STATE_READ, STATE_WRITE, STATE_ERROR, STATE_STDIN_PIPE, STATE_STDOUT_PIPE };

// the pool structure, the detailed one can be seen in csapp3e
struct Pool{
    int largest_fd;
    int n; // the length of current array
    int fd[FD_SIZE]; // array for colloecting all fds
    int nleft;
    int pipefd[FD_SIZE];
    fd_set ready_read_set;
    fd_set ready_write_set;
    fd_set read_set;
    fd_set write_set;
    struct rio_t rt[FD_SIZE];
    SSL *(CTX[FD_SIZE]);
    Request *rq[FD_SIZE];
    enum State state[FD_SIZE];
    struct Pipe_buf *(pipe_buf[FD_SIZE]);
};

struct CGI_POOL{
  int pipefd[FD_SIZE];
  int largest_fd;
  int n;
  int pool_i[FD_SIZE]; // the i of corresponding Pool
};

struct Pipe_buf {
  char buf[MAX_BODYSIZE];
  ssize_t len;
};

void init_pool(int, int);
int insert_pool(int,  SSL*);
int check_read_pool();
int check_write_pool();
int check_pipe_read_pool();
int insert_CGI_pool(int pipefd, int pool_i);
void init_CGI_pool();

#endif
