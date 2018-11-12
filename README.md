[![Build Status](https://travis-ci.org/Francesco149/oppai-ng.svg?branch=master)](https://travis-ci.org/Francesco149/oppai-ng)

difficulty and pp calculator for osu!

this is a pure C89 rewrite of
[oppai](https://github.com/Francesco149/oppai) with much lower
memory usage, smaller and easier to read codebase
executable size and better performance.

experimental taiko support is now available and appears to give
correct values for actual taiko maps. converted maps are still
unreliable due to incorrect slider conversion and might be
completely off (use ```-m1``` or ```-taiko``` to convert a std map
to taiko).

- [installing (linux)](#installing-linux)
- [installing (windows)](#installing-windows)
- [installing (osx)](#installing-osx)
- [usage](#usage)
- [implementations for other programming languages](#implementations-for-other-programming-languages)
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
sudo install -Dm 755 oppai /usr/bin/oppai

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

# implementations for other programming languages
oppai has been implemented for many other programming languages.
If you feel like making your own implementation and want it listed
here, open an issue or pull request. the requirement is that it
should pass the same test suite that oppai-ng passes.

note: these aren't just native bindings unless stated otherwise.

* [ojsama (javascript)](https://github.com/Francesco149/ojsama)
* [koohii (java)](https://github.com/Francesco149/koohii) . this
  is currently being used in tillerino.
* [pyttanko (python)](https://github.com/Francesco149/pyttanko)
* [oppai5 (golang)](https://github.com/flesnuk/oppai5) (by flesnuk)
* [OppaiSharp (C#)](https://github.com/HoLLy-HaCKeR/OppaiSharp)
  (by HoLLy)

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
    User time (seconds): 9.09
    System time (seconds): 0.06
    Percent of CPU this job got: 99%
    Elapsed (wall clock) time (h:mm:ss or m:ss): 0m 9.15s
    Average shared text size (kbytes): 0
    Average unshared data size (kbytes): 0
    Average stack size (kbytes): 0
    Average total size (kbytes): 0
    Maximum resident set size (kbytes): 11840
    Average resident set size (kbytes): 0
    Major (requiring I/O) page faults: 0
    Minor (reclaiming a frame) page faults: 304
    Voluntary context switches: 1
    Involuntary context switches: 39
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

the codebase has ~3-4x less lines than legacy oppai, making it easy
to read and use as a single header library. not only it is smaller,
but it now also implements both taiko and osu, so more features
than legacy oppai.

the osu! pp and diff calc alone would be around ~3k LOC including
the cli, which would be 5x less lines than legacy oppai for the
same functionality.

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

            Physical :  4123
              Source :  2906
             Comment :  492
 Single-line comment :  1
       Block comment :  491
               Mixed :  64
               Empty :  811
               To Do :  9

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

int main() {
  ezpp_t ez;
  ezpp_init(&ez);
  ez.mods = MODS_HD | MODS_DT;
  ezpp(&ez, "-");
  printf("%gpp\n", ez.pp);
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

if you don't feel comfortable writing bindings or using oppai
from c code, you can use the -o parameter to output in json or
other parsable formats. ```examples/binary.c``` shows how to parse
the binary output.

# shared library
you can also build oppai as a shared library with

```sh
./libbuild
```

this will generate a liboppai.so on linux/mac which you can copy to
```/usr/local/lib``` or anywhere in your library search paths

you can then use it by simply not defining ```OPPAI_IMPLEMENTATION``` .
this will exclude all the oppai code and just leave the header part


```c
#include "oppai.c"

int main() {
  /* ... */
}
```

then you can compile and run with

```
gcc test.c -lm -loppai
cat /path/to/file.osu | ./a.out
```

for windows you can use ```libbuild.bat``` to build (for details see the
info on compiling on windows) which will generate a oppai.dll and .lib pair

and then compile your program with msvc like so

```
cl test.c oppai.lib
```

then you can simply place the dll in the same folder as your executable
and run

# build parameters
when you build the oppai cli, you can pass any of these parameters
to the build script to disable features:

* ```-DOPPAI_UTF8GRAPH``` use utf-8 characters for the strains graph
* ```-DOPPAI_DEBUG``` enable debug output and debug output module

