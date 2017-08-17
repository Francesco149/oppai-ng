[![Build Status](https://travis-ci.org/Francesco149/oppai-ng.svg?branch=master)](https://travis-ci.org/Francesco149/oppai-ng)

difficulty and pp calculator for osu!

this is a pure C89 rewrite of
[oppai](https://github.com/Francesco149/oppai) with much lower
memory usage (~4-6x less), smaller and easier to read codebase
(~5 times less code), executable size (~7x smaller on linux),
~45% better performance and slightly faster compile time.

- [installing (linux)](#installing-linux)
- [installing (windows)](#installing-windows)
- [installing (osx)](#installing-osx)
- [usage](#usage)
- [oppai-ng vs old oppai](#oppai-ng-vs-old-oppai)
- [compile from source (windows)](#compile-from-source-windows)
- [using oppai as a library or making bindings](#using-oppai-as-a-library-or-making-bindings)
- [other build parameters](#other-build-parameters)

# installing (linux)
```sh
wget https://github.com/Francesco149/oppai-ng/archive/HEAD.tar.gz
tar xf HEAD.tar.gz
cd oppai-*
./build
sudo install -Dm oppai /usr/bin/oppai

oppai
```

you can also grab pre-compiled standalone binaries (statically
linked against musl libc) from
[here](https://github.com/Francesco149/oppai-ng/releases) if you
are somehow too scared to run those 5 commands.

# installing (windows)
download and unzip binaries from
[here](https://github.com/Francesco149/oppai-ng/releases) and
optionally add oppai's folder to your ```PATH``` environment
variable for easy access. you can find a guide
[here](https://www.howtogeek.com/118594/how-to-edit-your-system-path-for-easy-command-line-access/)
if you don't know how.

# installing (osx)
## via homebrew
```sh
brew install --HEAD pmrowla/homebrew-tap/oppai-ng
```
Note that installing with ```--HEAD``` is recommended but not required.
Installing from homebrew will place the ```oppai``` executable in your homebrew path.

## manually
Follow the same steps as for linux but substitute ```curl -O``` for ```wget``` since wget is not distributed by default in osx.
The same caveat applies if you want to run the test suite - you will need to edit the ```download_suite``` script to use curl.

# usage
you can run oppai with no arguments to check the documentation.

here's some example usages:

```sh
oppai path/to/map.osu +HDHR 98% 500x 1xmiss
oppai path/to/map.osu 3x100
oppai path/to/map.osu 3x100 OD10
oppai path/to/map.osu -ojson
```

you can also pipe maps from standard input by setting the filename
to ```-```.

for example on linux you can do:

```sh
curl https://osu.ppy.sh/osu/774965 | oppai - +HDDT
curl https://osu.ppy.sh/osu/774965 | oppai - +HDDT 1200x 1m
```

while on windows it's a bit more verbose (powershell):

```powershell
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai -
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai - +HDHR
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai - +HDHR 99% 600x 1m
```

I got the .osu file url from "Grab latest .osu file" on the
beatmap's page.

# oppai-ng vs old oppai
executable size is around 7 times smaller:
```sh
$ cd ~/src/oppai
$ ./build.sh -static
$ wc -c oppai
574648 oppai

$ cd ~/src/oppai-ng
$ ./build -static
$ wc -c oppai
75512 oppai
```

oppai-ng has proper error output in whatever format you select,
while legacy oppai either gives empty output or just dies with
a plaintext error.

oppai-ng has well-defined errno style error codes that you can
check for when using it as a library or reading its output.

the same test suite runs about 45% faster on oppai-ng compared
to old oppai, also the peak resident memory size is 4 to 6 times
smaller according to various ```time -v``` runs.

```sh
$ cd ~/src/oppai
$ ./build_test.sh
$ time -v ./oppai_test
...
    Command being timed: "./oppai_test"
    User time (seconds): 13.89
    System time (seconds): 0.10
    Percent of CPU this job got: 99%
    Elapsed (wall clock) time (h:mm:ss or m:ss): 0m 13.99s
    Average shared text size (kbytes): 0
    Average unshared data size (kbytes): 0
    Average stack size (kbytes): 0
    Average total size (kbytes): 0
    Maximum resident set size (kbytes): 45184
    Average resident set size (kbytes): 0
    Major (requiring I/O) page faults: 0
    Minor (reclaiming a frame) page faults: 2143
    Voluntary context switches: 1
    Involuntary context switches: 41
    Swaps: 0
    File system inputs: 0
    File system outputs: 0
    Socket messages sent: 0
    Socket messages received: 0
    Signals delivered: 0
    Page size (bytes): 4096
    Exit status: 0

$ cd ~/src/oppai-ng/test/
$ ./build
$ time -v ./oppai_test
...
    Command being timed: "./oppai_test"
    User time (seconds): 9.66
    System time (seconds): 0.05
    Percent of CPU this job got: 99%
    Elapsed (wall clock) time (h:mm:ss or m:ss): 0m 9.72s
    Average shared text size (kbytes): 0
    Average unshared data size (kbytes): 0
    Average stack size (kbytes): 0
    Average total size (kbytes): 0
    Maximum resident set size (kbytes): 11888
    Average resident set size (kbytes): 0
    Major (requiring I/O) page faults: 0
    Minor (reclaiming a frame) page faults: 305
    Voluntary context switches: 0
    Involuntary context switches: 73
    Swaps: 0
    File system inputs: 0
    File system outputs: 0
    Socket messages sent: 0
    Socket messages received: 0
    Signals delivered: 0
    Page size (bytes): 4096
    Exit status: 0
```

note that when the test suite is compiled without libcurl, the
resident memory usage drops by a flat 4mb, so almost half of that
is curl.

you can expect oppai memory usage to be under 4 mb most of the time
with the raw parsed beatmap data not taking more than ~800k even
for a 15 minute marathon.

the codebase has ~5x less lines than legacy oppai, making it easy
to read and use as a single header library

```sh
$ cd ~/src/oppai
$ sloc *.cc

---------- Result ------------

            Physical :  15310
              Source :  14406
             Comment :  301
 Single-line comment :  289
       Block comment :  12
               Mixed :  23
               Empty :  626
               To Do :  11

Number of files read :  10

------------------------------

$ cd ~/src/oppai-ng
$ sloc *.c

---------- Result ------------

            Physical :  3332
              Source :  2349
             Comment :  393
 Single-line comment :  1
       Block comment :  392
               Mixed :  49
               Empty :  659
               To Do :  6

Number of files read :  2

------------------------------
```

not to mention it's C89, which will be compatible with many more
platforms and old compilers than c++98

```oppai.c``` alone is only ~2200 LOC (~1500 without comments), and
you can compile piece of it out when you don't need them.

of course, it's not as heavily tested as legacy oppai (which runs
24/7 on Tillerino's back-end), however the test suite is a very
good test that runs through ~12000 unique scores and I'm confident
this rewrite is already very stable.

# compile from source (windows)
oppai should compile even on old versions of msvc dating back to
2005, although it was only tested on msvc 2010 and higher.

have at least [microsoft c++ build tools](http://landinghub.visualstudio.com/visual-cpp-build-tools)
installed. visual studio with c/c++ support also works.

open a visual studio prompt:

```bat
cd path\to\oppai\source
build.bat
oppai
```

you can also probably set up mingw and cygwin and follow the linux
instructions instead, I'm not sure. I don't use windows.

# using oppai as a library or making bindings
the new codebase is much easier to isolate and include in your
projects.

just copy oppai.c into your project, it acts as a single-header
library.

```c
#define OPPAI_IMPLEMENTATION
#include "../oppai.c"

int main()
{
    struct parser pstate;
    struct beatmap map;

    uint32_t mods;
    struct diff_calc stars;
    struct pp_calc pp;

    p_init(&pstate);
    p_map(&pstate, &map, stdin);

    mods = MODS_HD | MODS_DT;

    d_init(&stars);
    d_calc(&stars, &map, mods);
    printf("%g stars\n", stars.total);

    b_ppv2(&map, &pp, stars.aim, stars.speed, mods);
    printf("%gpp\n", pp.total);

    return 0;
}
```

```sh
gcc test.c
cat /path/to/file.osu | ./a.out
```

read oppai.c, there's documentation for each function at the top.

see examples directory for detailed examples. you can also read
main.c to see how the CLI uses it.

oppai is also modular, you can define out parts of the code
that you don't use by defining any of:
```
OPPAI_NOPARSER
OPPAI_NOPP
OPPAI_NODIFFCALC
```

if you don't feel comfortable writing bindings or using oppai
from c code, you can use the -o parameter to output in json or
other parsable formats. ```examples/binary.c``` shows how to parse
the binary output.

# other build parameters
when you build the oppai cli, you can pass any of these parameters
to the build script to disable features:

* ```-DOPPAI_NOTEXT``` disables text output module
* ```-DOPPAI_NOJSON``` disables json output module
* ```-DOPPAI_NOCSV``` disables CSV output module
* ```-DOPPAI_NOBINARY``` disables binary output module
* ```-DOPPAI_DEBUG``` enables debug output module and memory usage
    statistics
* ```-DOPPAI_NOSTDINT``` doesn't use ```stdint.h```, as some
    machines or old compilers don't have it

