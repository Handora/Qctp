/******************************************************************************
* loog.c                                                                      *
*                                                                             *
* Description: This function is used by Liso to response the request, and     *
*              some other other realated to response
    TODO:
        akk below are not implemted
*    1. Handle creation of a log file as specified in the "project handout.“  *
*    2. Make API functions for formatted (up to you; we may specify this      *
*       later) writing to this log file (not thread-safe).                    *
*    3. Make an API function for gracefully closing this log file.            *
*    4. Expose and use this module/API within the Checkpoints instead of      *
*       using stderr or stdout.                                               *
*    5. Log IP addresses and browser information/settings as coming from      *
*       connected clients.                                                    *
*    6. Log all errors encountered to the log file.                           *
*                                                                             *
* Authors: QianChen <qcdsr970209@gmail.com>                                   *
*                                                                             *
*******************************************************************************/
#include "qio.h"
#include <string.h>
#include "header.h"
#include <stdio.h>
#include <time.h>
#include "parse.h"
#include "loog.h"
#include "pool.h"
#include "ssl.h"
#include "get.h"
#include <errno.h>
#include <sys/stat.h>
#include "response.h"
#include <fcntl.h>
#include <unistd.h>
#include "get.h"
#include "post.h"
#include "head.h"
#include "util.h"
#include <ctype.h>
#include "cgi.h"

// extern WWW folder 定义在liso.c中作为全局变量。
extern char *WWW, *cgi_path;
extern struct Pool p;
extern struct CGI_POOL CGI_p;

/* @ Description: response the client with error-corresponding response
 * @ input:       client descriptor, error code, the reason of thus error
 * @ output:      if success respond the response and return 0, else
                  1), -1: no corresponding error。
                  2), -2: error when writing to clients。
 */
int
errorResponse(int connfd, SSL *CTX, char *code, char *cause, int close) {
  char buf[MAX_HEADSIZE], body[BUF_SIZE];
  char *msg;
  // get the current time
  time_t now = time(0);
  struct tm tm = *gmtime(&now);

  // judge the error code
  if (strcmp(code, "200") == 0) {
      msg = HTTP_200;
  } else if (strcmp(code, "404") == 0) {
      msg = HTTP_404;
  } else if (strcmp(code, "411") == 0) {
      msg = HTTP_411;
  } else if (strcmp(code, "500") == 0) {
      msg = HTTP_500;
  } else if (strcmp(code, "501") == 0) {
      msg = HTTP_501;
  } else if (strcmp(code, "503") == 0) {
      msg = HTTP_503;
  } else if (strcmp(code, "505") == 0) {
      msg = HTTP_505;
  } else {
      return -1; // error status code
  }

  // construct the error response
  sprintf(body, "<html><title>May be error hanppened\?\?\?</title>\r\n");
  sprintf(body, "%s<body>\r\n", body);
  sprintf(body, "%s<span>%s:%s</span><br/>\r\n", body, code, msg);
  sprintf(body, "%s<span>May be caused by %s</span><hr/>\r\n", body, cause);
  sprintf(body, "%s<em>Liso By Handora</em></body></html?\r\n", body);

  // response line
  sprintf(buf, "HTTP/1.1 %s %s\r\n", code, msg);
  if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
      return -2;
  }

  // Header: Connection
  if (close) {
      sprintf(buf, "Connection: close\r\n");
      if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
          return -2;
      }
  } else {
      sprintf(buf, "Connection: keep-alive\r\n");
      if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
          return -2;
      }
  }

  // Header: Date
  // when 500 or 503, Internal Date may be wrong
  if (strcmp(code, "500") && strcmp(code, "503")) {
    // use strftime to construct Date, the format is as foollowing
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    sprintf(buf, "Date: %s\r\n", buf);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
  }

  // Header: Server
  sprintf(buf, "Server: %s\r\n", SERVER);
  if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
      return -2;
  }

  // Header: Content-Length
  sprintf(buf, "Content-Length: %lu\r\n", strlen(body));
  if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
      return -2;
  }

  // Header: Content-Type
  sprintf(buf, "Content-Type: text/html\r\n\r\n");
  if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
      return -2;
  }

  // send the writen
  if (writen(connfd, CTX, body, strlen(body)) < 0) {
      return -2;
  }
  return 0;
}


/* @ Description: Serve the coming request
 * @ input:       client descriptor, Request structure and logfile
 * @ output:      if success respond the response and return 0, else
                  1), -1: no corresponding method or version,
                          they all handled in function
 */
int
server(int connfd, SSL *CTX, int i) {
  char errorMessage[BUF_SIZE];
  Request *request = p.rq[i];

  // whether it is legal in term of http version
  if (strcmp(request->http_version, "HTTP/1.1") != 0) {
      sprintf(errorMessage, "error in request: cann't recognize HTTP version");
      errorResponseAndCheck(connfd, CTX, "505", errorMessage, KEEP_ALIVE);
      return -1;
  }
  return serveUtility(connfd, CTX, i);
}

/* @ Description: Serve the coming GET
 * @ input:       client descriptor, Request structure and logfile
 * @ output:      if success respond the get response and return 0, else
                  1), -1: no corresponding file in the disk or have no privilege,
                          they all handled in function
 */
int
serveUtility(int connfd, SSL *CTX, int i) {
    char file[BUF_SIZE], *fileend, temp[BUF_SIZE];
    struct stat sbuf;
    char errorMessage[BUF_SIZE];
    Request* request = p.rq[i];
    // get the complete url

    size_t wwwLen = strlen(WWW);
    strncpy(file, WWW, wwwLen+1);
    fileend = strchr(request->http_uri, '?');
    if (fileend)
        strncat(file, request->http_uri, fileend-request->http_uri);
    else
        strncat(file, request->http_uri, BUF_SIZE-wwwLen-1);


    // if uri is '/', then strcat the index.html
    strcpy(temp, WWW);
    strcat(temp, "/");
    if (strcmp(file, temp) == 0) {
        strcat(file, "index.html");
    }

    // test the file and it's privilege
    if (stat(file, &sbuf) < 0) {
        sprintf(errorMessage, "error in stat: %s, %s", strerror(errno), file);
        errorResponseAndCheck(connfd, CTX, "404", errorMessage, KEEP_ALIVE);
        return -1;
    }

    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        sprintf(errorMessage, "error in stat: Not a regular file or not excutable");
        errorResponseAndCheck(connfd, CTX, "404", errorMessage, KEEP_ALIVE);
        return -1;
    } else {
        // compare the method and use the corresponding server
        if (strcasecmp(request->http_method, "GET") == 0) {
            return serveGet(connfd, CTX, file, &sbuf, request->close);
        } else if (strcasecmp(request->http_method, "HEAD") == 0) {
            return serveHead(connfd, CTX, file, &sbuf, request->close);
        } else if (strcasecmp(request->http_method, "POST") == 0) {
            if (request->content_length == -1) {
                sprintf(errorMessage, "error in request: post without content length");
                errorResponseAndCheck(connfd, CTX, "411", errorMessage, KEEP_ALIVE);
                return -1;
            }
            return servePost(connfd, CTX, file, &sbuf, request->close);
        } else {
            sprintf(errorMessage, "error in request: cann't recognize HTTP method");
            errorResponseAndCheck(connfd, CTX, "501", errorMessage, KEEP_ALIVE);
            return -1;
        }
    }
}

/* @ Description: get the file type through file name
 * @ input:       filename, a pointer to char for corresponding file type
 * @ output:      Null
 */
void
getType(char *filename, char *type) {
    // maybe some error?
    // more detail in header.h
    if (strstr(filename, ".html")) {
        strcpy(type, "text/html");
    } else if (endswith(filename, ".css")) {
        strcpy(type, "text/css");
    } else if (endswith(filename, ".png")) {
        strcpy(type, "image/png");
    } else if (endswith(filename, ".jpeg")) {
        strcpy(type, "image/jpeg");
    } else if (endswith(filename, ".gif")) {
        strcpy(type, "image/gif");
    } else if (endswith(filename, ".pdf")) {
        strcpy(type, "application/pdf");
    } else {
        strcpy(type, "text/plain");
    }
}

int
eatBody(struct rio_t *rp, SSL *CTX, char *req, int n) {
  int rc;
  char errorMessage[BUF_SIZE];

  if (n > MAX_BODYSIZE) {
    rc = rio_readn(rp, CTX, req, MAX_BODYSIZE);
    if (rc < 0) {
      sprintf(errorMessage, "rio_readn error when reading: %s", strerror(errno));
      errorResponseAndCheck(rp->rio_fd, CTX, "500", errorMessage, CLOSE);
      return -1;
    } else if (rc != MAX_BODYSIZE) {
      sprintf(errorMessage, "entity body length not equal to content-Length");
      errorResponseAndCheck(rp->rio_fd, CTX, "500", errorMessage, KEEP_ALIVE);
      return -2;
    }
    // return value more than 0 presents there are more value needed to be read
    return rc;
  }
  // for clean the last request's body to prevent memory leak
  rc = rio_readn(rp, CTX, req, n);

  if (rc < 0) {
    sprintf(errorMessage, "rio_readn error when reading: %s", strerror(errno));
    errorResponseAndCheck(rp->rio_fd, CTX, "500", errorMessage, CLOSE);
    return -1;
  } else if (rc != n) {
    sprintf(errorMessage, "entity body length not equal to content-Length");
    errorResponseAndCheck(rp->rio_fd, CTX, "500", errorMessage, KEEP_ALIVE);
    return -2;
  }
  return n;
}


void
closeRequest(int i) {
    if (p.rq[i]) {
        freeRequest(p.rq[i]);
        p.rq[i] = NULL;
    }
    if (p.CTX[i]) {
        close_ssl(p.CTX[i]);
        p.CTX[i] = NULL;
    }
    FD_CLR(p.fd[i], &p.ready_read_set);
    FD_CLR(p.fd[i], &p.ready_write_set);
    loogClosure(p.fd[i]);
    close_socket(p.fd[i]);

    p.fd[i] = -1;
}

void
errorResponseAndCheck(int fd, SSL *CTX, char *statusCode, char *message, int close) {
    char errorMessage[BUF_SIZE];
    int err;
    loogError(message);
    err = errorResponse(fd, CTX, statusCode, message, close);
    if (err == -2) {
        sprintf(errorMessage, "error in writen: %s", strerror(errno));
        loogError(errorMessage);
    }
}

char *
getFromRequestHeader(Request *request, char *name) {
  Request_header *h;
  for (h=request->headers; h; h=h->next) {
      if (strcasecmp(h->header_name, name) == 0) {
          return h->header_value;
      }
  }
  return 0;
}

int
eatHead(int i) {
  int len, ret;
  char Header[MAX_HEADSIZE];

  loog("== STATE READ HEADER ==");
  while (1) {
    len = rio_readlineb(&p.rt[i], p.CTX[i], Header, MAX_HEADSIZE);
    if (len < 0) {
      errorResponseAndCheck(p.fd[i], p.CTX[i], "400", "Bad Request", CLOSE);
      return -2;
    } else if (len == 0) {
      errorResponseAndCheck(p.fd[i], p.CTX[i], "400", "Bad Request", CLOSE);
      return -2;
    }

    // read over the header
    if (strcmp(Header, "\r\n") == 0) {
      p.state[i] = STATE_WRITE;
      FD_CLR(p.fd[i], &p.ready_read_set);
      FD_SET(p.fd[i], &p.ready_write_set);
      loog("== STATE REQUEST HEADER OK ==");
      // put the fd[i] in the writeable set
      break;
    }

    ret=parse(p.rq[i], Header, len);
    if (ret == 2) {
      continue;
    } else  {
      errorResponseAndCheck(p.fd[i], p.CTX[i], "400", "Bad Request", CLOSE);
      return -2;
    }
  }

  return 0;
}

int
eatLine(int i) {
  int len, ret;
  char Header[MAX_HEADSIZE], errorMessage[BUF_SIZE];

  loog("== STATE READ LINE ==");
  if (p.rq[i]) {
    freeRequest(p.rq[i]);
  }
  p.rq[i] = newRequest();
  len = rio_readlineb(p.rt+i, p.CTX[i], Header, MAX_HEADSIZE);
  if (len < 0) {
    sprintf(errorMessage, "Bad Request: %s", strerror(errno));
    errorResponseAndCheck(p.fd[i], p.CTX[i], "400", errorMessage, CLOSE);
    return -2;
  } else if (len == 0) {
    return -3;
  }
  ret=parse(p.rq[i], Header, len);
  if (ret != 1) {
    errorResponseAndCheck(p.fd[i], p.CTX[i], "400", "Bad Request3", CLOSE);
    return -2;
  }
  loog("== STATE READ LINE OK ==");
  return 0;
}

int
closePipefd(int i) {
  close(CGI_p.pipefd[i]);
  FD_CLR(CGI_p.pipefd[i], &p.ready_read_set);
  CGI_p.pipefd[i] = -1;
  CGI_p.pool_i[i] = -1;
  return 0;
}
