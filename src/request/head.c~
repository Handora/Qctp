#include "response.h"
#include "head.h"
#include <errno.h>
#include "loog.h"
#include <time.h>
#include "header.h"
#include <unistd.h>
#include <fcntl.h>

int
serveHead(int connfd, SSL *CTX, char *filename, struct stat* sbuf, int close) {
    //TODO: cgi
    int err, errno;
    char errorMessage[BUF_SIZE];

    err = headStatic(connfd, CTX, filename, sbuf, close);

    // writen return -2 has been handled
    if (err == -2) {
        sprintf(errorMessage, "error in writen: %s", strerror(errno));
        loogError(errorMessage);
    }
    return err;
}

int
headStatic(int connfd, SSL *CTX, char *filename, struct stat *sbuf, int close) {
    char buf[BUF_SIZE], tmp[BUF_SIZE];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);

    sprintf(buf, "HTTP/1.1 %s %s\r\n", "200", HTTP_200);
    if (writen(connfd, CTX, buf, strlen(buf)) < 0) {
        return -2;
    }
    if (close) {
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

    return 0;
}
