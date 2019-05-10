#ifndef PTY_H
#define PTY_H
#include <fcntl.h>

extern int PTYS[3];

extern pid_t pty_fork(int ptys[]);
extern pid_t pty_fork_exec(char *path, char *arg[]);

#endif // PTY_H
