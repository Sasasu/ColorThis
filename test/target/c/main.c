#include <stdio.h>
#include <unistd.h>

typedef struct {
  char *name;
  int fd;
} T;

int main() {
  T t[3] = {
      {.name = "STDIN", .fd = STDIN_FILENO},
      {.name = "STDOUT", .fd = STDOUT_FILENO},
      {.name = "STDERR", .fd = STDERR_FILENO},
  };

  int all_is_tty = 0;

  for (int i = 0; i < 3; i++) {
    if (isatty(t[i].fd)) {
      printf("%s is a tty\n", t[i].name);
    } else {
      all_is_tty = 1;
      printf("%s is not tty\n", t[i].name);
    }
  }

  return all_is_tty;
}
