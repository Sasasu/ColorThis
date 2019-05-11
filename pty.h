#ifndef PTY_H
#define PTY_H
#include <fcntl.h>
#include <termios.h>

extern int PTYS[3];

extern pid_t pty_fork(int ptys[]);
extern pid_t pty_fork_exec(char *path, char *arg[]);
extern void set_ter(int fd, tcflag_t flag);

#endif // PTY_H
