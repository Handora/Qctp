#ifndef UTIL_H
#define UTIL_H

#define MAX(x, y) ((x)>(y)?(x):(y))

int isInteger(char *num);
int startsWith(const char *str, const char *pre);
char *malloc_string(char *string);
int endswith(const char *str, const char *last);

#endif
