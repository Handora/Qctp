/*
 * The header of loog package
 * The detailed information
 * is in the loog.c file
 */

#ifndef LOOG_H
#define LOOG_H
#include <stdio.h>
#include "parser/parse.h"

int loogInit(char *filename);
void loog(char *message);
void loogConnection(char *ipAddr, int ipPort, int connfd);
void loogClosure(int connfd);
void loogrequest(int connfd, Request* request);
void loogResponse(int connfd);
void loogError(char *errorMessage);
#endif
