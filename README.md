# ColorThis

Tell the program I just want you to print the color.

## Quick start

### Prerequisites

the compile depends is

```
cmake make gcc
```

no runtime depends

### Installation

1. Download ColorThis use `git clone git@github.com:Sasasu/ColorThis.git`
2. Compile

```shell
cmake -DCMAKE_INSTALL_PREFIX=/usr .
make
sudo make install
```

## Usage

type ColorThis in your terminal, you will get like this

```shell
$ ColorThis
Usage: ColorThis <option> [application]
where options are
        -stdin:   make stdin is a tty
        -stdout:  make stdout is a tty
        -stderr:  make stderr is a tty
        -hook:    hook libc's isatty function
                  NOTE: may not work well
where application like
                  ls / </dev/null 2>/dev/null
```

this command shows full function

```
$ ColorThis -stdin -stdout -stderr $SOME_FRIENDLLY_GO_APP $COMMAND 2> colorfull_err_log.log < input_from_terminal.txt | tee colorfull_log.log
```

## How it works

if `-hook` is enabled, ColorThis will inject self to `LD_PRELOAD` and patch `isatty` function in libc.

but if the application call syscall directly or is fully static link this method will not work.

if `-hook` is not enabled, ColorThis will creat at most 3 ptys, catch all signs, creat a new session, fork and exec the new executable file.

ColorThis act like a terminal, exec the new executable file then attach some new tty to this session, these tty forward anything to the real tty.

## License

MIT
