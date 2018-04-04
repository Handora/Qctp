#include <stdio.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

int
serveGet(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close);

int
getStatic(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close);
