#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "log/loog.h"
#include "util/qio.h"

// rubust read from network
// read n bytes from fd into vptr
// and return the bytes readed
// if interruppted, return -1 expresses the error
ssize_t
readn(int fd, void *vptr, size_t n) {
  size_t nleft = n;
  ssize_t nread;
  char *ptr = (char *)vptr;

  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR) {
        nread = 0; //
      } else {
        return -1; // error happenes
      }
    } else if (nread == 0) {
      break;
    }
    nleft -= nread;
    ptr += nread;
  }
  return n - nleft;
}

ssize_t
writen(int fd, SSL* CTX, void *vptr, size_t n) {
  ssize_t nwrite;
  size_t nleft = n;
  char *ptr = (char *)vptr;

  while (nleft > 0) {
    if (CTX) {
      nwrite = SSL_write(CTX, ptr, nleft);
    } else {
      nwrite = write(fd, ptr, nleft);
    }
    if (nwrite <= 0) {
      if (nwrite < 0 && errno == EINTR) {
        nwrite = 0;
      } else {
        return -1; // error
      }
    }
    nleft -= nwrite;
    ptr += nwrite;
  }
  return n;
}

/* @ Description: safely close the socket desciptor
 * @ input:       sockfd and logFile stream
 * @ output:      if success return 0, else log the error and return 1
 * @ else:        the liso log the close_socket error in function
 *                in its function body
 */
int
close_socket(int sock) {
  char errorMessage[BUF_SIZE];
  if (close(sock)) {
    // the liso log the close_socket error in function in its function body
    sprintf(errorMessage, "Failed closing socket:%s", strerror(errno));
    loogError(errorMessage);
    return 1;
  }
  return 0;
}

/* @ Description: signal handle function in CSAPP3e
 * @ input:       signal and handler
 * @ output:      old signal handler
 */
handler_t *
Signal(int signum, handler_t *handler) {
  struct sigaction action, old_action;

  action.sa_handler = handler;
  sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
  action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

  if (sigaction(signum, &action, &old_action) < 0) {
    return 0;
  }

  return (old_action.sa_handler);
}

void
rio_readinitb(struct rio_t *rp, int fd) {
  rp->rio_fd = fd;
  rp->rio_bufptr = rp->rio_buf;
  rp->rio_cnt = 0;
}

ssize_t
rio_read(struct rio_t *rp, SSL *CTX, char *usrbuf, size_t n) {
  int cnt;

  while (rp->rio_cnt <= 0) {
    if (CTX)
      rp->rio_cnt = SSL_read(CTX, rp->rio_buf, sizeof(rp->rio_buf));
    else
      rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
    if (rp->rio_cnt < 0) {
      if (errno != EINTR) {
          return -1;
      }
    } else if (rp->rio_cnt == 0) {
      return 0;
    } else {
      rp->rio_bufptr = rp->rio_buf;
    }
  }

  cnt = n;
  if (rp->rio_cnt < n) {
    cnt = rp->rio_cnt;
  }
  memcpy(usrbuf, rp->rio_bufptr, cnt);
  rp->rio_bufptr += cnt;
  rp->rio_cnt -= cnt;
  return cnt;
}

ssize_t
rio_readlineb(struct rio_t *rp, SSL *CTX, void *buf, size_t maxline) {
  int n, rc;
  char c, *bufp = buf;

  for (n=0; n<maxline-1; n++) {
    if ((rc = rio_read(rp, CTX, &c, 1)) == 1) {
      *bufp++ = c;
      if (c == '\n') {
          n++;
          break;
      }
    } else if (rc == 0) {
      if (n == 0) {
          return 0;
      } else {
          break;
      }
    } else {
      return -1;
    }
  }
  *bufp = 0;
  return n;
}

ssize_t
rio_readn(struct rio_t *rp, SSL* CTX, void *buf, size_t maxn) {
  int n=maxn, rc;
  char *bufp = buf;

  while (n > 0) {
    if ((rc = rio_read(rp, CTX, bufp, n)) < 0) {
      return -1;
    } else if (rc == 0) {
      break;
    } else {
      n -= rc;
      bufp += rc;
    }
  }
  return maxn-n;
}

int
tcp_listen(const char *host, const char *serv, socklen_t *len) {
  struct addrinfo *ai, hints, *ai_head;
  char errorMessage[BUF_SIZE];
  int sockfd, on, n;

  bzero(&hints, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;

  if ((n=getaddrinfo(host, serv, &hints, &ai)) < 0) {
    snprintf(errorMessage, BUF_SIZE, "getaddrinfo error: %s", gai_strerror(n));
    loogError(errorMessage);
    exit(1);
  }
  ai_head = ai;

  do {
    if((sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
      continue;
    }

    on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
      close_socket(sockfd);
      continue;
    }

    if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) == 0) {
      snprintf(errorMessage, BUF_SIZE, "socket bind to %s", Sock_ntop(ai->ai_addr, ai->ai_addrlen));
      loog(errorMessage);
      break;
    }

    close_socket(sockfd);
  } while ((ai=ai->ai_next) != NULL);

  if (ai == NULL) {
    loogError("Can't bind to the host with server");
    exit(1);
  }

  if (listen(sockfd, 5) < 0) {
    loogError("Error in listenning");
    exit(1);
  }

  if (len) {
    *len = ai->ai_addrlen;
  }

  freeaddrinfo(ai_head);
  return sockfd;
}

char *
sock_ntop(const struct sockaddr *sa, socklen_t salen) {
  char		portstr[8];
  static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
/* end sock_ntop */

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		str[0] = '[';
		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
			return(str);
		}
		return (str + 1);
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

			/* OK to have no pathname bound to the socket: happens on
			   every connect() unless client calls bind() first. */
		if (unp->sun_path[0] == 0)
			strcpy(str, "(no pathname bound)");
		else
			snprintf(str, sizeof(str), "%s", unp->sun_path);
		return(str);
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

		if (sdl->sdl_nlen > 0)
			snprintf(str, sizeof(str), "%*s (index %d)",
					 sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
		else
			snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
		return(str);
	}
#endif
	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen) {
	char	*ptr;

	if ( (ptr = sock_ntop(sa, salen)) == NULL){
    loogError("error with sock_ntop");
  }
	return(ptr);
}
