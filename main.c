#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "epoll.h"
#include "hook.h"
#include "pty.h"

bool FAKE_FD[3] = {false, false, false};

void showhelp(char *name) {
  printf("Usage: %s <option> [application]\n", name);
  printf("where options are\n");
  printf("\t-stdin:   make stdin is a tty\n");
  printf("\t-stdout:  make stdout is a tty\n");
  printf("\t-stderr:  make stderr is a tty\n");
  printf("\t-hook:    hook libc's isatty function\n");
  printf("\t          NOTE: may not work well\n");
  printf("where application like\n");
  printf("\t          ls / </dev/null 2>/dev/null\n");
}

int main(int argc, char *argv[]) {
  int i;
  bool use_hook = false;

  if (argc == 1) {
    showhelp(argv[0]);
    exit(1);
  }

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-stdin"))
      FAKE_FD[STDIN_FILENO] = true;
    else if (!strcmp(argv[i], "-stdout"))
      FAKE_FD[STDOUT_FILENO] = true;
    else if (!strcmp(argv[i], "-stderr"))
      FAKE_FD[STDERR_FILENO] = true;
    else if (!strcmp(argv[i], "-hook"))
      use_hook = true;
    else
      break;
  }

  if (use_hook) {
    hook_exec(argv[i], argv + i);
    return 0;
  }

  pid_t pid = pty_fork_exec(argv[i], argv + i);

  epoll_init();
  start_loop();

  return waitpid(pid, NULL, 0);
}
