#include <stdio.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

int
servePost(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close);

int
postStatic(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close);
