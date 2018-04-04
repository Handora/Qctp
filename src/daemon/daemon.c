#include "log/loog.h"
#include "util/qio.h"
#include "unistd.h"
#include "signal.h"
#include "daemon/daemon.h"
#include <sys/stat.h>
#include <fcntl.h>

int
daemon_init() {
  pid_t pid;
  int i;

  if ((pid=fork()) < 0) {
    return -1;
  } else if (pid) {
    exit(0);
  }

  if (setsid() < 0) {
    return -1;
  }

  Signal(SIGHUP, SIG_IGN);

  if ((pid=fork()) < 0) {
    return -1;
  } else if (pid) {
    exit(0);
  }

  for (i=0; i<MAXFD; i++) {
    close(i);
  }

  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);

  return 0;
}
