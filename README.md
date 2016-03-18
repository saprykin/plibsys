
[![](https://api.travis-ci.org/saprykin/plib.svg?branch=master)](https://travis-ci.org/saprykin/plib)

======= About PLib =======

PLib is a cross-platform system C library with some helpful routines.
Do not confuse with PLIB which is using for game and 3D development.

======= Features =======

PLib gives you:

* Platform independent data types
* Threads: POSIX, Solaris and Win32
* Mutexes: POSIX, Solaris and Win32
* Conditional variables: POSIX, Solaris and Win32
* System-wide semaphores: POSIX, System V and Win32
* System-wide shared memory: POSIX, System V and Win32
* Atomic operations
* Sockets support (UDP, TCP, SCTP)
* Hash functions: MD5, SHA-1, GOST (R 34.11-94)
* Binary trees: BST, red-black, AVL
* INI files parser
* High resolution time profiler
* Files and directories
* Dynamic library loading
* Useful routines for linked lists, strings, hash tables
* Macros for OS and compiler detection
* Fully covered with unit tests

======= Platforms =======

PLib is a cross-platform, highly portable library, it was tested on
the following platforms:

* GNU/Linux
* OS X
* Windows
* Cygwin
* FreeBSD, NetBSD, OpenBSD
* DragonFlyBSD
* Solaris
* AIX
* HP-UX
* QNX Neutrino
* UnixWare 7
* SCO OpenServer 5

It should also work on other *nix systems with or without minimal
efforts.

======= Compilers =======

PLib was tested with the following compilers:

* MSVC (x32, x64) 2003 and above
* MinGW (x32, x64)
* Open Watcom (x32)
* GCC (x32, x64)
* Clang (x32, x64)
* Intel (x64)
* QCC
* Oracle Solaris Studio (x32, x64)

======= Building =======

Use CMake to build PLib for any target platform.

======= License =======

PLib is distributed under the terms of GNU GPLv2 license.
