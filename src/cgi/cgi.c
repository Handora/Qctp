#include <openssl/ssl.h>
#include "cgi/cgi.h"
#include <unistd.h>
#include "log/loog.h"
#include "event/pool.h"
#include "util/qio.h"
#include "util/util.h"
#include <errno.h>
#include "request/response.h"
#include <sys/socket.h>
#include <arpa/inet.h>

extern struct Pool p;
extern char *WWW, *cgi_path;
extern struct CGI_POOL CGI_p;

int
serveCGI(int i) {
  char errorMessage[BUF_SIZE], file[BUF_SIZE];
  struct stat sbuf;

  strcpy(file, cgi_path);

  if (strcmp(p.rq[i]->http_version, "HTTP/1.1") != 0) {
      sprintf(errorMessage, "error in request: cann't recognize HTTP version");
      errorResponseAndCheck(p.fd[i], p.CTX[i], "505", errorMessage, KEEP_ALIVE);
      return -1;
  }

  if (stat(file, &sbuf) < 0) {
      sprintf(errorMessage, "error in stat: %s, %s", strerror(errno), file);
      errorResponseAndCheck(p.fd[i], p.CTX[i], "404", errorMessage, KEEP_ALIVE);
      return -1;
  }

  if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      sprintf(errorMessage, "error in stat: Not a regular file or not excutable");
      errorResponseAndCheck(p.fd[i], p.CTX[i], "404", errorMessage, KEEP_ALIVE);
      return -1;
  }
  return send_CGI(i);
}


int
send_CGI(int i) {
  int pid, j, n, nread;
  char errorMessage[BUF_SIZE];
  char *ARGS[2] = {
    NULL,
    NULL
  };
  char *EVRN[4096];
  int stdin_pipe[2], stdout_pipe[2];
  char http_body[300];

  char file[BUF_SIZE];
  strcpy(file, cgi_path);
  ARGS[0] = file;

  if (pipe(stdin_pipe) < 0) {
    sprintf(errorMessage, "Error piping for stdin.\n");
    loogError(errorMessage);
    closeRequest(i);
    return -3;
  }
  if (pipe(stdout_pipe) < 0) {
    sprintf(errorMessage, "Error piping for stdout.\n");
    loogError(errorMessage);
    // we need kill the unwanted stdin_pipe and stdout pipe for not descriptor leak
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    closeRequest(i);
    return -3;
  }

  pid = fork();
  if (pid < 0) {
    sprintf(errorMessage, "fork error %s", strerror(errno));
    loog(errorMessage);
    // we need kill the unwanted stdin_pipe and stdout pipe for not descriptor leak
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    closeRequest(i);
    return -3;
  } else if (pid == 0) {
    // when the sub process exit, EVRN will be freed implicitly
    if (constructCGIEnviron(EVRN, i) < 0) {
      sprintf(errorMessage, "constructEVRN error");
      loog(errorMessage);
      exit(0);
    }

    if (dup2(stdin_pipe[1], fileno(stdout)) < 0) {
      sprintf(errorMessage, "dup2 error %s", strerror(errno));
      loog(errorMessage);
      exit(0);
    }
    if (dup2(stdout_pipe[0], fileno(stdin)) < 0) {
      sprintf(errorMessage, "dup2 error %s", strerror(errno));
      loog(errorMessage);
      exit(0);
    }

    // kill all unwanted socket
    for (j=0; j< p.n; j++) {
      if (p.fd[j] > 0) {
        if (STDIN_FILENO != p.fd[j] && STDOUT_FILENO != p.fd[j]) {
          // shouldn't check the return value of close here, otherwhise it will be caused to exit and no reply!!!
          close(p.fd[j]);
        }
      }
    }
    for (j=0; j<CGI_p.n; j++) {
      if (CGI_p.pipefd[j] > 0) {
        if (STDIN_FILENO != CGI_p.pipefd[j] && STDOUT_FILENO != CGI_p.pipefd[j]) {
          // shouldn't check the return value of close here, otherwhise it will be caused to exit and no reply!!!
          close(CGI_p.pipefd[j]);
        }
      }
    }
    // kill the ssl_sockfd and sockfd
    close(4);close(5);
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);

    if (execve(ARGS[0], ARGS, EVRN)) {
      sprintf(errorMessage, "execve error %s", strerror(errno));
      loog(errorMessage);
      exit(0);
    }
  } else if (pid > 0) {
    int nleft = p.rq[i]->content_length;
    if (nleft < 0 && strcasecmp(p.rq[i]->http_method, "post") == 0) {
      sprintf(errorMessage, "error in request: post without content length");
      errorResponseAndCheck(p.fd[i], p.CTX[i], "411", errorMessage, KEEP_ALIVE);
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);
      close(stdout_pipe[0]);
      close(stdout_pipe[1]);
      return -4;
    } else if (nleft < 0) {
      nleft = 0;
    }

    while (nleft > 0) {
      nread=eatBody(p.rt+i, p.CTX[i], http_body, nleft);
      n = writen(stdout_pipe[1], NULL, http_body, nread);
      if (n <= 0) {
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return -5;
      }
      nleft-=nread;
    }

    // read error
    if (nleft < 0) {
      switch(nleft) {
      case -1:
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return -5;
      default:
        return -4;
      }
    }

    if (insert_CGI_pool(stdin_pipe[0], i) < 0) {
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);
      close(stdout_pipe[0]);
      close(stdout_pipe[1]);
      return -4;
    }
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
  }
  return 0;
}

int
constructCGIEnviron(char **ENVP, int i) {
  char *res;
  int cnt=0;
  char buf[BUF_SIZE], tmp[BUF_SIZE];

  // Content-Length
  res = getFromRequestHeader(p.rq[i], "content-length");
  if (res == 0) {
      res = getFromRequestHeader(p.rq[i], "content_length");
  }

  if (res) {
      snprintf(buf, BUF_SIZE, "CONTENT_LENGTH=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "CONTENT_LENGTH=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // Content-Type
  res = getFromRequestHeader(p.rq[i], "content-type");
  if (res == 0) {
      res = getFromRequestHeader(p.rq[i], "content_type");
  }
  if (res) {
      snprintf(buf, BUF_SIZE, "CONTENT_TYPE=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "CONTENT_TYPE=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // GATEWAY_INTERFACE
  snprintf(buf, BUF_SIZE, "GATEWAY_INTERFACE=CGI/1.1");
  ENVP[cnt++] = malloc_string(buf);

  // QUERY_STRING
  res = strchr(p.rq[i]->http_uri, '?');
  if (res) {
      snprintf(buf, BUF_SIZE, "QUERY_STRING=%s", res+1);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "QUERY_STRING=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // PATH_INFO
  if(res) {
      strncpy(tmp, p.rq[i]->http_uri+4, res-p.rq[i]->http_uri-4);
      tmp[res-p.rq[i]->http_uri-4] = '\0';
      snprintf(buf, BUF_SIZE, "PATH_INFO=%s", tmp);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "PATH_INFO=%s", p.rq[i]->http_uri+4);
      ENVP[cnt++] = malloc_string(buf);
  }

  // SCRIPT_NAME
  snprintf(buf, BUF_SIZE, "SCRIPT_NAME=/cgi");
  ENVP[cnt++] = malloc_string(buf);

  // TODO:REMOTE_ADDR
  // if (getpeername(p, (struct sockaddr *)&sockAddr, &addrLen) < 0) {
  //     sprintf(errorMessage, "Error getpeername: %s", strerror(errno));
  //     loog(errorMessage);
  //     return -1;
  // }
  // snprintf(buf, BUF_SIZE, "REMOTE_ADDR=%s", inet_ntoa(sockAddr.sin_addr));
  // ENVP[cnt++] = malloc_string(buf);

  // REQUEST_METHOD
  snprintf(buf, BUF_SIZE, "REQUEST_METHOD=%s", p.rq[i]->http_method);
  ENVP[cnt++] = malloc_string(buf);

  // REQUEST_URI
  snprintf(buf, BUF_SIZE, "REQUEST_URI=%s", p.rq[i]->http_uri);
  ENVP[cnt++] = malloc_string(buf);

  // SERVER_PORT
  if (p.CTX[i]) {
      snprintf(buf, BUF_SIZE, "SERVER_PORT=443");
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "SERVER_PORT=80");
      ENVP[cnt++] = malloc_string(buf);
  }

  // SERVER_PROTOCOL
  snprintf(buf, BUF_SIZE, "SERVER_PROTOCOL=HTTP/1.1");
  ENVP[cnt++] = malloc_string(buf);

  // SERVER_SOFTWARE
  snprintf(buf, BUF_SIZE, "SERVER_SOFTWARE=Liso/1.0");
  ENVP[cnt++] = malloc_string(buf);

  // HTTP_ACCEPT
  res = getFromRequestHeader(p.rq[i], "accept");
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_REFERER
  res = getFromRequestHeader(p.rq[i], "referer");
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_REFERER=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_REFERER=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_ACCEPT_ENCODING
  res = getFromRequestHeader(p.rq[i], "accept-encoding");
  if (res == 0) {
      res = getFromRequestHeader(p.rq[i], "accept_encoding");
  }
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT_ENCODING=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT_ENCODING=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_ACCEPT_LANGUAGE
  res = getFromRequestHeader(p.rq[i], "accept-language");
  if (res == 0) {
      res = getFromRequestHeader(p.rq[i], "accept_language");
  }
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT_LANGUAGE=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT_LANGUAGE=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_ACCEPT_CHARSET
  res = getFromRequestHeader(p.rq[i], "accept-charset");
  if (res == 0) {
      res = getFromRequestHeader(p.rq[i], "accept_charset");
  }
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT_CHARSET=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_ACCEPT_CHARSET=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_HOST
  res = getFromRequestHeader(p.rq[i], "host");
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_HOST=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_HOST=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_COOKIE
  res = getFromRequestHeader(p.rq[i], "cookie");
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_COOKIE=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_COOKIE=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_USER_AGENT
  res = getFromRequestHeader(p.rq[i], "user-agent");
  if (res == 0) {
      res = getFromRequestHeader(p.rq[i], "user_agent");
  }
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_USER_AGENT=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_USER_AGENT=");
      ENVP[cnt++] = malloc_string(buf);
  }

  // HTTP_CONNECTION
  res = getFromRequestHeader(p.rq[i], "connection");
  if (res) {
      snprintf(buf, BUF_SIZE, "HTTP_CONNECTION=%s", res);
      ENVP[cnt++] = malloc_string(buf);
  } else {
      snprintf(buf, BUF_SIZE, "HTTP_CONNECTION=");
      ENVP[cnt++] = malloc_string(buf);
  }

  return cnt;
}
