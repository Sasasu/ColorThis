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
static pid_t pid;

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

void setup_sig(void);

int main(int argc, char *argv[]) {
  int i;
  bool use_hook = false;

  if (argc == 1) {
    showhelp(argv[0]);
    exit(1);
  }

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-stdin")) {
      set_ter(STDIN_FILENO, ~ICANON & ~ECHO);
      FAKE_FD[STDIN_FILENO] = true;
    } else if (!strcmp(argv[i], "-stdout"))
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

  pid = pty_fork_exec(argv[i], argv + i);

  setup_sig();
  epoll_init();
  start_loop();

  return waitpid(pid, NULL, 0);
}

void sig_handler(int signo) {
  switch (signo) {
  case SIGCHLD:
    exit(waitpid(pid, NULL, 0));
  default:
    kill(pid, signo);
  }
}

static struct sigaction sa;
void setup_sig(void) {
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = &sig_handler;
  sigfillset(&sa.sa_mask);
  if (sigaction(SIGCHLD, &sa, NULL) == -1 || // sub process exit
      sigaction(SIGINT, &sa, NULL) == -1     // C-c
  ) {
    perror("handle signal");
    exit(1);
  }
}
