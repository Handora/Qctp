#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "request/post.h"
#include "request/response.h"
#include "request/header.h"
#include "log/loog.h"

int
servePost(int connfd, SSL *CTX, char *filename, struct stat* sbuf, int close) {
    //TODO: cgi
    int err, errno;
    char errorMessage[BUF_SIZE];

    err = postStatic(connfd, CTX, filename, sbuf, close);

    // writen return -2 has been handled
    if (err == -2) {
        sprintf(errorMessage, "error in writen: %s", strerror(errno));
        loogError(errorMessage);
    }
    return err;
}

int
postStatic(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int closes) {
    char buf[BUF_SIZE], errorMessage[BUF_SIZE], tmp[BUF_SIZE];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    int f;
    ssize_t n;
    sprintf(buf, "HTTP/1.1 %s %s\r\n", "200", HTTP_200);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
    if (closes) {
        sprintf(buf, "Connection: close\r\n");
    } else {
        sprintf(buf, "Connection: keep-alive\r\n");
    }
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
    strftime(tmp, sizeof(tmp), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    sprintf(buf, "Date: %s\r\n", tmp);
    writen(connfd, CTX, buf, strlen(buf));
    sprintf(buf, "Server: %s\r\n", SERVER);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
    sprintf(buf, "Content-Length: %lu\r\n", sbuf->st_size);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
    getType(filename, tmp);
    sprintf(buf, "Content-Type: %s\r\n", tmp);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
    tm = *gmtime(&sbuf->st_mtime);
    strftime(tmp, sizeof(tmp), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    sprintf(buf, "Last-Modified: %s\r\n\r\n", tmp);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }

    if ((f = open(filename, O_RDONLY)) < 0) {
        sprintf(errorMessage, "error in open: %s", strerror(errno));
        errorResponseAndCheck(connfd, CTX, "404", errorMessage, KEEP_ALIVE);
        return -1;
    }

    while ((n = read(f, buf, sizeof(buf))) > 0) {
        if (writen(connfd, CTX, buf, n) < 0) {
            return -2;
        }
    }

    if (n < 0) {
        sprintf(errorMessage, "error in read: %s", strerror(errno));
        errorResponseAndCheck(connfd, CTX, "500", errorMessage, CLOSE);
        return -1;
    }

    if (close(f) < 0) {
        sprintf(errorMessage, "error in close: %s", strerror(errno));
        loogError(errorMessage);
    }
    return 0;
}
