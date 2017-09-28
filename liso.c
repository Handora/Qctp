/******************************************************************************
* liso.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>                          *
*                                                                             *
*******************************************************************************/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include "qio.h"
#include "pool.h"
#include "loog.h"
#include "ssl.h"
#include <errno.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/stat.h>
#include "util.h"
#include "daemon.h"

char *WWW, *cgi_path;
struct Pool p;
struct CGI_POOL CGI_p;

int
main(int argc, char* argv[]) {
  int sock, client_sock, ssl_sock, ret;
  socklen_t cli_size;
  struct sockaddr_in cli_addr;
  char *httpPort, *httpsPort;
  char *privateKey, *certificate;
  char errorMessage[BUF_SIZE];
  SSL_CTX *ssl_context;
  SSL * client_context;

  // Liso will always have 8 arguments—functional or not
  // if not, print the usage, and exit
  if (argc != 9) {
    fprintf(stderr, "Usage: ./lisod <HTTP port> <HTTPS port> <log file> <lock file> <www folder> \
<CGI script path> <private key file> <certificate file>\n");
    return EXIT_FAILURE;
  }

#ifdef PRODUCTION
  if (daemon_init() < 0) {
    fprintf(stderr, "daemon_init error\n");
    return EXIT_FAILURE;
  }
#endif

  // get information from argv
  httpPort = argv[1];
  httpsPort = argv[2];
  if(loogInit(argv[3]) < 0) {
    fprintf(stdout, "log init error\n");
    return EXIT_FAILURE;
  }
  WWW = argv[5];
  cgi_path = argv[6];
  privateKey = argv[7];
  certificate = argv[8];

  // handle the SIGPIPE with SIG_IGN, forbidden the broken pipe
  // which is writen in UNIX NETWORK PROGRAMMING Chapter 5
  if (Signal(SIGPIPE, SIG_IGN) < 0) {
    sprintf(errorMessage, "error in Signal: %s", strerror(errno));
    loogError(errorMessage);
    return EXIT_FAILURE;
  }

  // init the ssl sock for future use
  if ((ssl_context = sslInit(privateKey, certificate)) == NULL) {
    return EXIT_FAILURE;
  }
  if ((ssl_sock = tcp_listen(NULL, httpsPort, NULL)) < 0) {
    return EXIT_FAILURE;
  }

  // Listen and serve function, if error, we just exit,
  // because function inside has log it
  if ((sock = tcp_listen(NULL, httpPort, NULL)) < 0) {
    return EXIT_FAILURE;
  }

  // init the pool
  init_pool(sock, ssl_sock);
  init_CGI_pool();

  /* finally, loop waiting for input and then write it back */
  while (1) {
   p.read_set = p.ready_read_set; // init the set
   p.write_set = p.ready_write_set;

   // block until one of read set can be read
   ret = select(MAX(CGI_p.largest_fd, p.largest_fd)+1, &(p.read_set), &(p.write_set), 0, 0);
   if (ret < 0) {
       // switch the errno to judge which error it is and handle it
       switch (errno) {
        // EINTR should be IGNORE and recover the errno
        case EINTR:
            errno = 0;
            continue;
        default:
            sprintf(errorMessage, "Error select: %s", strerror(errno));
            loogError(errorMessage);
            // exit will close all useless descriptor
            return EXIT_FAILURE;
       }
   }

   if (FD_ISSET(sock, &(p.read_set))) {
     cli_size = sizeof(cli_addr);
     if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr, &cli_size)) <= 0) {
       // switch the errno to judge which error it is and handle it
       switch (errno) {
        // maybe some error, currently it may be ok
        case ECONNABORTED:
        case EINTR:
        case EPROTO:
          errno = 0;
          continue;
        default:
          sprintf(errorMessage, "accepting error: %s", strerror(errno));
          loogError(errorMessage);
          // exit will close all useless descriptor
          return EXIT_FAILURE;
       }
     }

     // then insert in the pool, this function handle the error in its body
     if (insert_pool(client_sock, NULL) == 0) {
       loogConnection(inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, client_sock);
     }
   }

   if (FD_ISSET(ssl_sock, &(p.read_set))) {
       cli_size = sizeof(cli_addr);
       if ((client_sock = accept(ssl_sock, (struct sockaddr *) &cli_addr, &cli_size)) <= 0) {
         // switch the errno to judge which error it is and handle it
         switch (errno) {
          // maybe some error, currently it may be ok
          case EPROTO:
          case ECONNABORTED:
          case EINTR:
            errno = 0;
            continue;
          default:
            sprintf(errorMessage, "accepting error: %s", strerror(errno));
            loogError(errorMessage);
            close_socket(ssl_sock);
            return EXIT_FAILURE;
         }
       }

       if ((client_context = SSL_new(ssl_context)) == NULL) {
         sprintf(errorMessage, "SSL_new error: %s", strerror(errno));
         loogError(errorMessage);
         close_socket(client_sock);
         continue;
       }

       if (SSL_set_fd(client_context, client_sock) == 0) {
         sprintf(errorMessage, "SSL_set_id error: %s", strerror(errno));
         loogError(errorMessage);
         SSL_free(client_context);
         close_socket(client_sock);
         continue;
       }

       int ret;
       if ((ret=SSL_accept(client_context)) <= 0) {
         sprintf(errorMessage, "Error accepting (handshake) client SSL context %d",  SSL_get_error(client_context, ret));
         loogError(errorMessage);
         SSL_free(client_context);
         close_socket(client_sock);
         continue;
       }

       // then insert in the pool, this function handle the error in its body
       if (insert_pool(client_sock, client_context) == 0) {
         loogConnection(inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, client_sock);
       }
   }
   // find the select pool for something to read, write ot some exception
   check_read_pool();
   check_pipe_read_pool();
   check_write_pool();
  }
}
