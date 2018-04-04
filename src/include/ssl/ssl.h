#include <openssl/ssl.h>

SSL_CTX *
sslInit(char *privateKey, char *certificate);

void
close_ssl(SSL *CTX);
