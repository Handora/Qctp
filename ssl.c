#include "qio.h"
#include "loog.h"
#include <openssl/ssl.h>

SSL_CTX *
sslInit(char *privateKey, char *certificate) {
  /************ VARIABLE DECLARATIONS ************/
  SSL_CTX *ssl_context;
  /************ END VARIABLE DECLARATIONS ************/

  /************ SSL INIT ************/
  SSL_load_error_strings();
  SSL_library_init();

  /* we want to use TLSv1 only */
  if ((ssl_context = SSL_CTX_new(TLSv1_server_method())) == NULL)
  {
    loogError("Error conducting ssl context");
    return NULL;
  }

  /* register private key */
  if (SSL_CTX_use_PrivateKey_file(ssl_context, privateKey,
                                  SSL_FILETYPE_PEM) == 0)
  {
    SSL_CTX_free(ssl_context);
    loogError("Error associating private key.");
    return NULL;
  }

  /* register public key (certificate) */
  if (SSL_CTX_use_certificate_file(ssl_context, certificate,
                                   SSL_FILETYPE_PEM) == 0)
  {
    SSL_CTX_free(ssl_context);
    loogError("Error associating certificate.");
    return NULL;
  }

  return ssl_context;
}

void
close_ssl(SSL *CTX) {
  if (CTX) {
    SSL_free(CTX);
  }
  loog("Close the HTTPS connection");
}
