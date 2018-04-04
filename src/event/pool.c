#include "event/pool.h"
#include "util/qio.h"
#include "log/loog.h"
#include "request/response.h"
#include "ssl/ssl.h"
#include "cgi/cgi.h"
#include "request/get.h"
#include "request/post.h"
#include "request/head.h"
#include "cgi/cgi.h"
#include "util/util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

extern char *cgi_path;
extern struct Pool p;
extern struct CGI_POOL CGI_p;

void
init_CGI_pool() {
  int i;
  for (i=0; i<FD_SIZE; i++) {
    CGI_p.pipefd[i] = -1;
    CGI_p.pool_i[i] = -1;
    CGI_p.n = 0;
    CGI_p.largest_fd = -1;
  }
}

int
insert_CGI_pool(int pipefd, int pool_i) {
  /* code */
  int i;
  for (i=0; i<FD_SIZE; i++) {
    if (CGI_p.pipefd[i] < 0) {
      CGI_p.pipefd[i] = pipefd;
      if (i > CGI_p.n) {
        CGI_p.n = i;
      }
      if (pipefd > CGI_p.largest_fd) {
        CGI_p.largest_fd = pipefd;
      }
      CGI_p.pool_i[i] = pool_i;
      p.state[pool_i] = STATE_STDIN_PIPE;
      FD_CLR(p.fd[pool_i], &p.ready_read_set);
      FD_SET(pipefd, &p.ready_read_set);
      p.pipefd[pool_i] = pipefd;
      break;
    }
  }

  if (i == FD_SIZE) {
    loogError("Over the FD_SIZE(fd) of pipe: fd");
    return -1;
  }

  return 0;
}

/* @ Description: init the pool strcuture to initialize the select pool
 * @ input:       server sock descriptor and empty pool structure
 * @ output:      null
 */
void
init_pool(int sock, int ssl_sock) {
  // initialize the pool, the detail principle is in CSAPP3e
  int i;

  p.largest_fd = sock;
  p.n = 0;
  p.nleft = 0;
  memset(p.CTX, 0, sizeof(p.CTX));
  for (i=0; i<FD_SIZE ;i++) {
      p.rq[i] = 0;
      p.fd[i] = -1;
      p.pipefd[i] = -1;
      p.pipe_buf[i] = 0;
  }
  FD_ZERO(&(p.ready_read_set));
  FD_ZERO(&(p.read_set));
  FD_ZERO(&(p.ready_write_set));
  FD_ZERO(&(p.write_set));
  FD_SET(sock, &(p.ready_read_set));
  FD_SET(ssl_sock, &(p.ready_read_set));
}

/* @ Description: insert one fd in pool sttucture
 * @ input:       the descriptor you want to insert in the pool, the pool structure and logfile
 * @ output:      if success return 0.
                  else if the pool is filled return -1, handled
 */
int
insert_pool(int fd, SSL *CTX) {
  int i;

  // tranverse the pool to look for inserting into the pool
  for (i = 0; i < FD_SIZE; i++) {
    // should also update the pool, the detail is in csapp3e
    if (p.fd[i] < 0) {
      p.fd[i] = fd;
      if (CTX) {
          p.CTX[i] = CTX;
      } else {
          p.CTX[i] = 0;
      }
      if (fd > p.largest_fd) {
          p.largest_fd = fd;
      }
      if (i + 1 > p.n) {
          p.n = i + 1;
      }
      p.state[i] = STATE_READ;
      FD_SET(fd, &(p.ready_read_set));
      rio_readinitb(&(p.rt[i]), fd);
      break;
    }
  }

  // the pool is filled
  if (i == FD_SIZE) {
    loogError("Over the FD_SIZE(fd): fd");
    close_socket(fd);
    return -1;
  }
  return 0;
}

/* @ Description: process the pool to read request
 * @ input:       pool structure and log file
 * @ output:      if success return 0, error are handled in function self
 */
int
check_read_pool() {
  int i, ret;
  char temp[MAX_BODYSIZE];

  // transver the pool to look for the one can read from
  for (i = 0; (i < p.n+1); i++) {
    if (p.fd[i] > 0 && FD_ISSET(p.fd[i], &(p.read_set))) {
      if (p.state[i] == STATE_READ) {
        // read line
        if ((ret = eatLine(i)) < 0) {
          switch (ret) {
          case -2:
          case -3:
            closeRequest(i);
            continue;
          }
        }
        // read line ok

        // read head
        if ((ret = eatHead(i)) < 0) {
          switch (ret) {
          case -2:
            closeRequest(i);
            continue;
          }
        }
        // reaa head OK

        if (strstr(p.rq[i]->http_uri, "cgi")) {
          loog("== STATE CGI PIPE ==");

          if ((ret=serveCGI(i)) < 0) {
            switch (ret) {
            case -4:
              p.state[i] = STATE_READ;
              continue;
            case -5:
              closeRequest(i);
              continue;
            default:
              p.state[i] = STATE_ERROR;
              continue;
            }
          }
          loog("== STATE CGI PIPE OK ==");
        }
      }

      if(p.rq[i] && p.state[i] == STATE_ERROR) {
        loog("STATE ERROR HANDLE");

        // handle the redundant post body if needed
        // TODO:
        //   may be something wrong here
        if (p.rq[i] && strcasecmp(p.rq[i]->http_method, "post") == 0) {
          loog("STATE ERROR READ BODY");
          int nleft = p.rq[i]->content_length;
          while ((nleft=eatBody(p.rt+i, p.CTX[i], temp, nleft)) > 0);
        }
        closeRequest(i);
        continue;
      }
    }
  }
  return 0;
}

int
check_pipe_read_pool() {
  int i, n;
  int pool_i;
  char buf[MAX_BODYSIZE];

  for (i=0; i<CGI_p.n+1; i++) {
    pool_i = CGI_p.pool_i[i];
    if (CGI_p.pipefd[i] > 0 && FD_ISSET(CGI_p.pipefd[i], &(p.read_set)) && p.state[pool_i] == STATE_STDIN_PIPE) {
      loog("== STATE STDIN PIPE ==");
      n=read(CGI_p.pipefd[i], buf, MAX_BODYSIZE);
      if (n == 0) {
        FD_SET(p.fd[pool_i], &p.ready_read_set);
        closePipefd(i);
        p.state[pool_i] = STATE_READ;
      } else if (n < 0) {
        FD_SET(p.fd[pool_i], &p.ready_read_set);
        closePipefd(i);
        p.state[pool_i] = STATE_READ;
      } else {
        p.pipe_buf[pool_i] = (struct Pipe_buf*)malloc(sizeof(struct Pipe_buf));
        memcpy(p.pipe_buf[pool_i]->buf, buf, n);
        p.pipe_buf[pool_i]->len = n;
        FD_CLR(CGI_p.pipefd[i], &p.ready_read_set);
        FD_SET(p.fd[pool_i], &p.ready_write_set);
        p.state[pool_i] = STATE_WRITE;
      }
      loog("== STATE STDIN PIPE OK ==");
    }
  }
  return 0;
}

int
check_write_pool() {
  int i, ret;

  for (i=0; i<p.n+1; i++) {
    if (p.fd[i] > 0 && FD_ISSET(p.fd[i], &(p.write_set)) && p.state[i] == STATE_WRITE) {
      loog("== state write ==");
      if (startsWith(p.rq[i]->http_uri, "/cgi")) {
        writen(p.fd[i], p.CTX[i], p.pipe_buf[i]->buf, p.pipe_buf[i]->len);
        FD_CLR(p.fd[i], &(p.ready_write_set));
        FD_SET(p.pipefd[i], &(p.ready_read_set));
        p.state[i] = STATE_STDIN_PIPE;
        free(p.pipe_buf[i]);
        p.pipe_buf[i] = NULL;
      } else {
        if ((ret=server(p.fd[i], p.CTX[i], i))<0) {
          switch(ret) {
          case -1:
            FD_SET(p.fd[i], &(p.ready_read_set));
            p.state[i] = STATE_READ;
          case -2:
            closeRequest(i);
          }
          FD_CLR(p.fd[i], &(p.ready_write_set));
          continue;
        }
        FD_CLR(p.fd[i], &(p.ready_write_set));
        FD_SET(p.fd[i], &(p.ready_read_set));
        p.state[i] = STATE_READ;
      }
      loog("== state write OK ==");
    }
  }
  return 0;
}
