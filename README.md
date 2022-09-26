## About

`xvp` executes programs against NUL-separated argument list in file.

`xvp` is NOT replacement for `xargs` or `xe` - it does the same tasks with different options.

`xvp` is small supplemental utility for [minimal Debian container image](https://github.com/rockdrilla/docker-debian) (*work in progress*).

Program is kinda *dumb* so feel free to open issue/PR. :)

---

## Usage:

`xvp [-u] <program> [..<common args>] <arg file>`

`<arg file>` - file with NUL-separated arguments; symlinks are NOT supported (for good reason)

### Options:

| Option | Description                             |
| ------ | --------------------------------------- |
|  `-u`  | unlink (delete) `<arg file>` after work |


### Example:

```sh
printf '%s\0' $(seq 1 6) > /tmp/argz ; xxd < /tmp/argz ; xvp -u -- sh -c 'echo $@ ; echo ; exit 7' -- /tmp/argz ; echo $? ; ls -l /tmp/argz
00000000: 3100 3200 3300 3400 3500 3600            1.2.3.4.5.6.
1 2 3 4 5 6

7
/usr/bin/ls: cannot access '/tmp/argz': No such file or directory
```

Generic case:

```sh
xvp program ./argfile
```

is roughly equal to:

```sh
xargs -0 -a ./argfile program &
wait ; rm -f ./argfile
```

## Building from source:

Note: clone repository **recursively** (due to `git submodule`)

```sh
git clone --recurse-submodules https://github.com/rockdrilla/xvp.git
```

Build dependencies: `gcc` and `libc6-dev`.

*They're quite common for building software from sources.*

```sh
gcc -O2 -o xvp xvp.c
```

---

## License

Apache-2.0

- [spdx.org](https://spdx.org/licenses/Apache-2.0.html)
- [opensource.org](https://opensource.org/licenses/Apache-2.0)
- [apache.org](https://www.apache.org/licenses/LICENSE-2.0)
