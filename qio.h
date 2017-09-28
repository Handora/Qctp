#ifndef QIO_H
#define QIO_H
#include <stdio.h>
#include <stdint.h>
#include <openssl/ssl.h>
#include <sys/socket.h>

#define BUF_SIZE 4196
#define RIO_BUFSIZE 8192
#define MAX_HEADSIZE 8192

typedef void handler_t(int);
#define MIN(x, y) (((x)<(y)) ? (x) : (y))

struct rio_t {
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
};

ssize_t
readn(int fd, void *vptr, size_t n);

ssize_t
writen(int fd, SSL *CTX, void *vptr, size_t n);

int
close_socket(int sock);

handler_t *
Signal(int signum, handler_t *handler);

void
rio_readinitb(struct rio_t *rp, int fd);

ssize_t
rio_read(struct rio_t *rp, SSL *CTX, char *usrbuf, size_t n);

ssize_t
rio_readlineb(struct rio_t *rp, SSL *CTX, void *buf, size_t maxline);

ssize_t
rio_readn(struct rio_t *rp, SSL* CTX, void *buf, size_t maxn);

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen);

char *
sock_ntop(const struct sockaddr *sa, socklen_t salen);

int
tcp_listen(const char *host, const char *serv, socklen_t *len);

#endif
