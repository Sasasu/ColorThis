#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern bool FAKE_FD[3];
static char PRELOAD[2048] = "libLibColorThis.so";

__attribute__((noreturn)) void hook_exec(char *path, char *arg[]) {
  if (FAKE_FD[STDIN_FILENO] == true)
    setenv("FAKE_STDIN", "1", 0);
  if (FAKE_FD[STDOUT_FILENO] == true)
    setenv("FAKE_STDOUT", "1", 0);
  if (FAKE_FD[STDERR_FILENO] == true)
    setenv("FAKE_STDERR", "1", 0);

  char *old_preload = getenv("LD_PRELOAD");
  if (old_preload != NULL && strlen(old_preload) != 0) {
    size_t old_len = strlen(PRELOAD);
    PRELOAD[old_len] = ':';
    PRELOAD[old_len + 1] = '\0';
    strcat(PRELOAD + old_len + 1, old_preload);
  }

  setenv("LD_PRELOAD", PRELOAD, 1);
  execvp(path, arg);
  perror("exec");
  exit(1);
}
