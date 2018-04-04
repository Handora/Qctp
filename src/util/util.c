#include <ctype.h>
#include <string.h>
#include <stdlib.h>

int
isInteger(char *num) {
  int i, n = strlen(num);

  for (i=0; i<n; i++) {
    if (!isdigit(num[i])) {
      return 0;
    }
  }
  return 1;
}

int
startsWith(const char *str, const char *pre) {
  return strncmp(str, pre, strlen(pre)) == 0;
}

int
endswith(const char *str, const char *last) {
  int i, j;
  for (i=strlen(last)-1, j=strlen(str)-1; i>=0 && j>=0; i--, j--) {
    if (str[j] != last[i]) {
      return 0;
    }
  }
  if (i < 0) {
    return 1;
  } else {
    return 0;
  }
}

char *
malloc_string(char *string) {
  int len = strlen(string);
  char *str = (char *)malloc(len + 1);
  strcpy(str, string);
  str[len] = 0;
  return str;
}
