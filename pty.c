#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pty.h"

extern int posix_openpt(int); // posix_openpt in libc
extern int grantpt(int);      // grantt in libc
extern int unlockpt(int);     // unlockpt in libc
extern char *ptsname(int);    // ptsname in libc

// 0 => don't use
// -1 => use this pty
int PTYS[3] = {0, 0, 0};

int ptym_open(char *ptyname) {
  int psfd = posix_openpt(O_RDWR);
  char *pty;

  if (psfd < 0)
    goto err_ptym;
  if (grantpt(psfd) < 0)
    goto err_ptym;
  if (unlockpt(psfd) < 0)
    goto err_ptym;
  if ((pty = ptsname(psfd)) == NULL)
    goto err_ptym;

  strcpy(ptyname, pty);
  return psfd;

err_ptym:
  perror(ptyname);
  return -1;
}

int ptys_open(char *ptyname) {
  int fd = open(ptyname, O_RDWR);
  if (fd < 0)
    goto err_ptys;

  return fd;

err_ptys:
  perror(ptyname);
  return -1;
}

pid_t pty_fork(int ptys[]) {
  pid_t pid;
  char pty_names[3][20];

  for (size_t i = 0; i < 3; i++) {
    if (ptys[i] == 0)
      continue;
    ptys[i] = ptym_open(pty_names[i]);
    if (i == STDIN_FILENO) {
      set_ter(ptys[STDIN_FILENO], ~ICANON & ~ECHO);
    }
    if(i == STDERR_FILENO){
        set_ter(ptys[STDERR_FILENO], ~ICANON);
    }
  }

  if ((pid = fork()) < 0) {
    perror("fork");
    return -1;
  } else if (pid == 0) { // child
    if (setsid() < 0) {
      perror("setsid");
      return -1;
    }
    for (size_t i = 0; i < 3; i++) {
      if (ptys[i] == 0)
        continue;

      int tmp_fd = ptys[i];
      if ((ptys[i] = ptys_open(pty_names[i])) < 0) {
        return -1;
      }
      close(tmp_fd);

      if (dup2(ptys[i], (int)i) != (int)i) {
        perror("dup2");
        return -1;
      }
      close(ptys[i]);
    }

    return 0;
  }
  // parent
  return pid;
}

extern bool FAKE_FD[3]; // in main.c

int pty_fork_exec(char *path, char *arg[]) {
  pid_t pid;
  for (size_t i = 0; i < 3; i++) {
    if (FAKE_FD[i] == true) {
      PTYS[i] = -1;
    }
  }

  if ((pid = pty_fork(PTYS)) < 0) {
    return -1;
  } else if (pid == 0) {
    execvp(path, arg);
  }
  return pid;
}

void set_ter(int fd, tcflag_t flag) {
  if (!isatty(fd))
    return;
  struct termios t;
  tcgetattr(fd, &t);
  t.c_lflag &= flag;
  tcsetattr(fd, TCSANOW, &t);
}
