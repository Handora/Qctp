#include "parse.h"

Request *
newRequest() {
    Request *request = (Request *) malloc(sizeof(Request));
    request->header_count=0;
    request->content_length = -1;
    request->close = 0;
    request->headers = 0;
    return request;
}

/**
* Given a char buffer returns the parsed request headers
*/
int
parse(Request *request, char *buffer, int size) {
	set_parsing_options(buffer, size, request);
    int ret;

	if ((ret = yyparse()) == PARSELINE) {
        return 1;
    } else if (ret == PARSEHEADER) {
        return 2;
    }
    return 0;
}

void
addHeader(Request *request, char *name, char *value) {
    Request_header *header = malloc(sizeof(Request_header));
    strncpy(header->header_name, name, 4096);
    strncpy(header->header_value, value, 4096);
    header->next = request->headers;
    request->headers = header;
    request->header_count ++;
    if (strcasecmp(name, "connection") == 0) {
        if (strcasecmp(value, "close") == 0) {
            request->close = 1;
        } else {
            request->close = 0;
        }
        return ;
    }
    if(strcasecmp(name, "content-length") == 0) {
        request->content_length = atoi(value);
        return;
    }
    if(strcasecmp(name, "content_length") == 0) {
        request->content_length = atoi(value);
        return;
    }
}

void
freeRequest(Request *request) {
    Request_header *h = request->headers, *tmp;
    while (h) {
        tmp = h;
        h = h->next;
        free(tmp);
    }
    if(request)
        free(request);
}

Request *
copyRequest(Request *request) {
    Request *req = (Request *) malloc(sizeof(Request));
    Request_header *h, *p;
    req->header_count=request->header_count;
    req->content_length = request->content_length;
    req->close = request->close;
    req->headers = 0;
    /*
    char http_version[50];
    char http_method[50];
    char http_uri[4096];
    */
    strcpy(req->http_method, request->http_method);
    strcpy(req->http_uri, request->http_uri);
    strcpy(req->http_version,request->http_version);

    for (h=request->headers; h; h=h->next) {
        p = (struct Request_header *)malloc(sizeof(struct Request_header));
        p->next = req->headers;
        strcpy(p->header_name, h->header_name);
        strcpy(p->header_value, h->header_value);
        req->headers = p;
    }

    return req;
}
