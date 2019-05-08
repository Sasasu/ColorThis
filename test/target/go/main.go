package main

import (
	"fmt"
	"os"

	"golang.org/x/sys/unix"
)

type T struct {
	name string
	fd   uintptr
}

func isatty(fd uintptr) bool {
	_, err := unix.IoctlGetTermios(int(fd), unix.TCGETS)
	return err == nil
}

func main() {
	t := [3]T{
		T{"STDIN", os.Stdin.Fd()},
		T{"STDOUT", os.Stdout.Fd()},
		T{"STDERR", os.Stderr.Fd()},
	}

	var all_is_tty = true

	for _, v := range t {
		if isatty(v.fd) {
			fmt.Println(v.name, "is a TTY")
		} else {
			all_is_tty = false
			fmt.Println(v.name, "not a TTY")
		}
	}

	if !all_is_tty {
		os.Exit(1)
	}
}
