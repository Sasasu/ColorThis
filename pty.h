#ifndef PTY_H
#define PTY_H
#include <fcntl.h>

extern pid_t pty_fork(int ptys[]);
extern pid_t pty_fork_exec(char *path, char *arg[]);
extern int ptys[3];
#endif // PTY_H
