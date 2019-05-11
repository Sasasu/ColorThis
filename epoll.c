#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "epoll.h"
#include "pty.h"

/*
if $IN_FD can epoll, when $IN_FD ready, read from $IN_FD and write to $OUT_FD
if $IN_FD can not epoll, when $OUT_FD ready, read from $IN_FD to $OUT_FD
if read EOF from $IN_FD, close $OUT_FD and remove from poll
 */

#define MAX_EVENT 3

extern bool FAKE_FD[3]; // in main.c

static int epollfd = 0;
static int fd_sum = 0;
static struct epoll_event ev, events[MAX_EVENT];

void set_nonblocking(int fd) {
  int flags;

  if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
    flags = 0;
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    perror("set nonblock");
    exit(1);
  }
}

int epoll_init() {
  epollfd = epoll_create1(EPOLL_CLOEXEC);
  if (epollfd < 0) {
    perror("epoll");
    exit(1);
  }

  // for stdin
  if (FAKE_FD[STDIN_FILENO] == true) {
    assert(PTYS[STDIN_FILENO] != 0 && PTYS[STDIN_FILENO] != -1);
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    // try add STDIN_FILENO into epoll, if failed try to add ptys into epoll
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
      ev.events = EPOLLIN;
      ev.data.fd = PTYS[STDIN_FILENO];
      if (epoll_ctl(epollfd, EPOLL_CTL_ADD, PTYS[STDIN_FILENO], &ev) < 0) {
        perror("epoll add");
        exit(1);
      } else {
        // set in pty no blocking
        fd_sum++;
        set_nonblocking(PTYS[STDIN_FILENO]);
      }
    } else {
      // STDIN_FILENO added into epoll, set STDIN_FILENO NOBLOCKING
      fd_sum++;
      set_nonblocking(STDIN_FILENO);
    }
  }

  // for stdout and stderr
  for (int i = 1; i < 3; i++) {
    if (FAKE_FD[i] != true)
      continue;
    assert(PTYS[i] != 0 && PTYS[i] != -1);
    // in this case, pty should can be epoll
    ev.events = EPOLLIN;
    ev.data.fd = PTYS[i];
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, PTYS[i], &ev) < 0) {
      perror("epoll add");
      exit(1);
    } else {
      fd_sum++;
      set_nonblocking(PTYS[i]);
    }
  }

  return 0;
}

void fd_action(int fd) {
  static char BUFFER[1024 * 1024 * 8]; // 8MiB
  ssize_t n;
  int read_fd, write_fd;

  if (fd == STDIN_FILENO) {
    read_fd = STDIN_FILENO;
    write_fd = PTYS[STDIN_FILENO];
  } else if (fd == PTYS[STDIN_FILENO]) {
    read_fd = STDIN_FILENO;
    write_fd = PTYS[STDIN_FILENO];
  } else if (fd == PTYS[STDOUT_FILENO]) {
    read_fd = PTYS[STDOUT_FILENO];
    write_fd = STDOUT_FILENO;
  } else if (fd == PTYS[STDERR_FILENO]) {
    read_fd = PTYS[STDERR_FILENO];
    write_fd = STDERR_FILENO;
  } else {
    fprintf(stderr, "error");
    exit(1);
  }

  n = read(read_fd, BUFFER, sizeof(BUFFER));
  if (n < 0 || n == 0) {
    ev.events = EPOLLOUT;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
    fd_sum--;
  } else {
    if (n != write(write_fd, BUFFER, (size_t)n)) {
      perror("write");
      exit(1);
    }
  }
}

void start_loop() {
  while (fd_sum != 0) {
    int nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
    if (nfds == -1) {
      if (errno == EINTR)
        continue;
      perror("epoll_wait");
      exit(1);
    }
    for (size_t n = 0; n < (size_t)nfds; n++) {
      fd_action(events[n].data.fd);
    }
  }
}
