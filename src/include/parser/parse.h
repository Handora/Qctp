#ifndef PARSE_H
#define PARSE_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PARSELINE 0
#define PARSEHEADER 1

//Header field
typedef struct Request_header
{
	char header_name[4096];
	char header_value[4096];
	struct Request_header *next;
} Request_header;

//HTTP Request Header
typedef struct
{
	char http_version[50];
	char http_method[50];
	char http_uri[4096];
	Request_header *headers;
	int header_count;
  int close;
  int content_length;
} Request;

Request *newRequest();
int parse(Request *request, char *buffer, int size);
void addHeader(Request *request, char *name, char *value);
void freeRequest(Request *request);
Request *copyRequest(Request *request);

#endif
