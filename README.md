hcasl
=====

Command line tool for "head -c N &amp;&amp; shift 1 byte/char" loop.

License
-------

zlib License.

Target environments
-------------------

Windows, Linux, macOS.

Probably it works fine on other OS.

Set up
------

hcasl is written in several programming languages. Choose one among them.

### For ANSI C89/C99 implementation:

1. Compile hcasl.c. Use make and Makefile.
2. Put hcasl in a directory registered in PATH.

| toolset                            | Makefile                 |
|:-----------------------------------|:-------------------------|
| Linux                              | Makefile                 |
| Mac OS X                           | Makefile\_mac            |
| MinGW/TDM-GCC (with GNU make)      | Makefile\_mingw          |
| MinGW-w64/TDM64-GCC (32bit binary) | Makefile\_mingw64\_32bit |

### For C++11 implementation:

1. Compile hcasl.cpp. Use make and Makefile.
2. Put hcasl in a directory registered in PATH.

| toolset                            | Makefile                 |
|:-----------------------------------|:-------------------------|
| Linux                              | Makefile                 |
| Mac OS X                           | Makefile\_mac            |
| MinGW/TDM-GCC (with GNU make)      | Makefile\_mingw          |
| MinGW-w64/TDM64-GCC (32bit binary) | Makefile\_mingw64\_32bit |

### For D (DMD 2.069.0) implementation:

1. Install [DMD](http://dlang.org/ "Official website") 2.069.0 or later.
2. Compile hcasl.d (`dmd hcasl.d`).
3. Put hcasl in a directory registered in PATH.

### For Go 1.4 implementation:

1. Install [Go](https://golang.org/ "Official website") 1.4.2 or later.
2. Compile hcasl.go (`go build hcasl.go`).
3. Put hcasl in a directory registered in PATH.
4. (Optional) Compile hcasl-char.go, and then put hcasl-char in a directory registered in PATH.

### For JVM/Groovy implementation:

1. Install [Groovy](http://www.groovy-lang.org/ "Official website") 2.4.5 or later.
2. Put hcasl in a directory registered in PATH.
3. (Optional) Put hcasl-char in a directory registered in PATH.

### For Perl 5 implementation:

1. Install Perl 5.14.2 or later.
2. Put hcasl in a directory registered in PATH.

### For Python 2.x implementation:

1. Install Python 2.7.13 or later.
2. Put hcasl in a directory registered in PATH.

### For Python 3.x implementation:

1. Install Python 3.8.10 or later.
2. Put hcasl in a directory registered in PATH.

### For Ruby implementation:

1. Install Ruby 1.9.3p0 or later.
2. Put hcasl in a directory registered in PATH.
3. (Optional) Put hcasl-char in a directory registered in PATH.

### For Scheme/Gauche implementation:

1. Install [Gauche](http://practical-scheme.net/gauche/ "Official website") 0.9.4 or later.
2. Put hcasl in a directory registered in PATH.
3. (Optional) Put hcasl-char in a directory registered in PATH.

### For Shell/bash implementation:

1. Install bash 4.2.25 or later.
2. Put hcasl in a directory registered in PATH.
3. (Optional) `ln -s hcasl hcasl-char`.
4. (Optional) Put hcasl-posix-like in a directory registered in PATH.
5. (Optional) `ln -s hcasl-posix-like hcasl-char-posix-like`.

### For Shell/sh implementation:

1. Install Bourne shell variants.
    * BusyBox 1.18.5 or later.
    * bash 4.2.25 or later.
    * dash 0.5.7 or later.
2. Install AWK variants.
    * BusyBox 1.18.5 or later.
    * Gawk 3.1.8 or later.
    * mawk 1.3.3 or later.
3. Put hcasl in a directory registered in PATH.

### For Tcl 8.6 implementation:

1. Install Tcl 8.6.4-5 or later.
2. Put hcasl-char in a directory registered in PATH.
3. (Optional) Put hcasl-char-recur in a directory registered in PATH.

Usage
-----

Please check help message `hcasl -h` or `hcasl-char -h`

Example
-------

    $ echo -n '' | hcasl
    $ echo -n 1234567 | hcasl
    $ echo -n 12345678 | hcasl
    12345678
    $ echo -n 123456789 | hcasl
    12345678
    23456789
    $ echo -n abc | hcasl -n 1
    a
    b
    c
    $ echo -n abc | hcasl -n 2
    ab
    bc
    $ echo -n abc | hcasl -n 3
    abc
    $ echo -n abc | hcasl -n 4
    $ echo -n 123456789 | hcasl
    12345678
    23456789
    $ echo -n 123456789 | hcasl -o output.txt
    $ cat output.txt
    12345678
    23456789
    $ _
