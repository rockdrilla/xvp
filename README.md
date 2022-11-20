## About

`xvp` executes program against NUL-separated argument list in file.

If argument list (file) is empty then program will not be executed - compare with "`xargs -r`" and consider this behavior as default.

Hovewer, `xvp` is NOT replacement for `xargs` or `xe` - it does the same tasks with different options.

`xvp` is small supplemental utility for [minimal Debian container image](https://github.com/rockdrilla/docker-debian) (*work in progress*).

If you have problems with it then feel free to open the issue/PR. :)

---

## Usage:

`xvp [-a <arg0>] [-cfiu] <program> [..<common args>] <arg file>`

`<arg file>` - file with NUL-separated arguments

### Options:

| Option       | Description                                                               |
| ------       | ------------------------------------------------------------------------- |
|  `-i`        | information only (print related system limits and do not run `<program>`) |
|  `-a <arg0>` | set `argv[0]` for `<program>` to `<arg0>`                                 |
|  `-c`        | run `<program>` with empty environment                                    |
|  `-f`        | force **single** `<program>` execution or return error                    |
|  `-u`        | unlink (delete) `<arg file>` after work only if it's regular file         |

### Example:

#### Example 1 - common

Nota bene: two dashes ("--") are mandatory for /bin/sh in this example.

```sh
# printf '%s\0' $(seq 1 6) > /tmp/argz
# xxd < /tmp/argz
00000000: 3100 3200 3300 3400 3500 3600            1.2.3.4.5.6.
# xvp -u sh -c 'echo $@ ; echo ; exit 15' -- /tmp/argz
1 2 3 4 5 6

# echo $?
15
# ls -l /tmp/argz
ls: cannot access '/tmp/argz': No such file or directory
```

#### Example 2 - option "-a"

Nota bene: if option "`-a`" is specified then next argument is treated as `<arg0>` and used as is.

Consider having test program `printargs` with source code:

```c
#include <stdio.h>
int main(int argc, char * argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
}
```

Invoke it with clean environment:

```sh
# printf '%s\0' arg1 arg2 > /tmp/argz
# xxd < /tmp/argz
00000000: 6172 6731 0061 7267 3200                 arg1.arg2.
# xvp -c printargs /tmp/argz
printargs arg1 arg2
```

Now add "`-a testapp`"

```sh
# xvp -a testapp -c printargs /tmp/argz
testapp arg1 arg2
```

And finally - remove "*by mistake*" value "`testapp`" after option "`-a`"

```sh
# xvp -a -c printargs /tmp/argz
-c arg1 arg2
```

Option "`-c`" was not in effect in `xvp`.

This is why option "`-a`" should be used with care.

#### Example 3 - analog in shell

Generic case "`xvp -u program ./argfile`" is roughly equal to:

```sh
#!/bin/sh
xargs -0 -a ./argfile program &
wait ; rm -f ./argfile
```

## Building from source:

Note: clone repository **recursively** (due to `git submodule`)

```sh
git clone --recurse-submodules https://github.com/rockdrilla/xvp.git
```

Build dependencies: `binutils`, `gcc`, `libc6-dev` and `make`.

*They're quite common for building software from sources.*

Then run `make` (recipe is [here](Makefile)).

```sh
make
```

---

## License

Apache-2.0

- [spdx.org](https://spdx.org/licenses/Apache-2.0.html)
- [opensource.org](https://opensource.org/licenses/Apache-2.0)
- [apache.org](https://www.apache.org/licenses/LICENSE-2.0)
