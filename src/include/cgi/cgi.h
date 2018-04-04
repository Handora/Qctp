#ifndef CGI_H
#define CGI_H

#include <openssl/ssl.h>
#include "parser/parse.h"

int serveCGI(int i);
int constructCGIEnviron(char **ENVP, int i);
int send_CGI(int i);

#endif
