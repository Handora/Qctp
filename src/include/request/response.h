// details are in response.c
#ifndef RESPONSE_H
#define RESPONSE_H
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/ssl.h>
#include <unistd.h>

#include "parser/parse.h"
#include "util/qio.h"
#include "event/pool.h"


#define CLOSE 1
#define KEEP_ALIVE 0

int errorResponse(int connfd, SSL *CTX, char *code, char *cause, int close);
int server(int connfd, SSL *CTX, int i);
int serveStatic(int connfd, char *filename, struct stat *sbuf);
void getType(char *filename, char *type);
int serveUtility(int connfd, SSL *CTX, int i);
int serverContection(Request *request);
int serverContentLength(Request *request);
int eatBody(struct rio_t *rp, SSL *CTX, char *req, int n);
int eatHead(int i);
int eatLine(int i);
void closeRequest(int i);
void errorResponseAndCheck(int fd, SSL *CTX, char *statusCode, char *message, int close);
char *getFromRequestHeader(Request *request, char *value);
int closePipefd(int i);
#endif
