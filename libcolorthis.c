#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int (*orig_isatty)(int) = NULL;
static bool FAKE_FD[3] = {false, false, false};

int isatty(int fd) {
  if (fd == STDIN_FILENO)
    return FAKE_FD[STDIN_FILENO] ? true : orig_isatty(fd);
  if (fd == STDOUT_FILENO)
    return FAKE_FD[STDOUT_FILENO] ? true : orig_isatty(fd);
  if (fd == STDERR_FILENO)
    return FAKE_FD[STDERR_FILENO] ? true : orig_isatty(fd);

  return orig_isatty(fd);
}

__attribute__((constructor)) static void setup(void) {
  if (getenv("FAKE_STDIN") != NULL)
    FAKE_FD[STDIN_FILENO] = true;
  if (getenv("FAKE_STDOUT") != NULL)
    FAKE_FD[STDOUT_FILENO] = true;
  if (getenv("FAKE_STDERR") != NULL)
    FAKE_FD[STDERR_FILENO] = true;

  void *libc = dlopen("libc.so.6", RTLD_LAZY);
  if (libc == NULL) {
    fprintf(stderr, "dlopen(): %s", dlerror());
    exit(1);
  }

  orig_isatty = (int (*)(int))dlsym(libc, "isatty");
  if (orig_isatty == NULL) {
    fprintf(stderr, "dlsym(): %s", dlerror());
    exit(1);
  }
}
