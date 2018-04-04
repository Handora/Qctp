#include <stdio.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

int
serveHead(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close);

int
headStatic(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close);
