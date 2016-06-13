
[![](https://api.travis-ci.org/saprykin/plibsys.svg?branch=master)](https://travis-ci.org/saprykin/plibsys)
[![](https://ci.appveyor.com/api/projects/status/github/saprykin/plibsys?branch=appveyor_test&svg=true)](https://ci.appveyor.com/project/saprykin/plibsys)
[![](https://scan.coverity.com/projects/8333/badge.svg)](https://scan.coverity.com/projects/saprykin-plibsys)
[![](https://codecov.io/gh/saprykin/plibsys/branch/master/graph/badge.svg)](https://codecov.io/gh/saprykin/plibsys)
[![](https://codedocs.xyz/saprykin/plibsys.svg)](https://codedocs.xyz/saprykin/plibsys/)
[![](http://img.shields.io/:license-lgpl2-blue.svg?style=flat)](http://www.gnu.org/licenses/lgpl-2.1.html)

======= About plibsys =======

plibsys is a cross-platform system C library with some helpful routines.

======= Features =======

plibsys gives you:

* Platform independent data types
* Threads: POSIX, Solaris and Win32
* Mutexes: POSIX, Solaris and Win32
* Condition variables: POSIX, Solaris and Win32
* Read-write locks: POSIX, Solaris and Win32
* System-wide semaphores: POSIX, System V and Win32
* System-wide shared memory: POSIX, System V and Win32
* Optimized spinlock
* Atomic operations
* Socket support (UDP, TCP, SCTP)
* Hash functions: MD5, SHA-1, GOST (R 34.11-94)
* Binary trees: BST, red-black, AVL
* INI file parser
* High resolution time profiler
* Files and directories
* Shared library loading
* Useful routines for linked lists, strings, hash tables
* Macros for OS and compiler detection

======= Platforms =======

plibsys is a cross-platform, highly portable library, it is fully covered
with unit tests and was tested on the following platforms:

* GNU/Linux
* OS X
* Windows
* Cygwin
* FreeBSD, NetBSD, OpenBSD
* DragonFlyBSD
* Solaris
* AIX
* HP-UX
* IRIX64
* QNX Neutrino
* UnixWare 7
* SCO OpenServer 5
* Haiku

It should also work on other *nix systems with or without minimal efforts.

======= Compilers =======

plibsys was tested with the following compilers:

* MSVC (x86, x64) 2003 and above
* MinGW (x86, x64)
* Open Watcom (x86)
* Borland (x86)
* GCC (x86, x64, PPC32be, PPC64be, IA-64/32, IA-64, HPPA2.0-32, MIPS32, AArch32, SPARCv9)
* Clang (x86, x64, PPC32be)
* Intel (x86, x64)
* QCC (x86)
* Oracle Solaris Studio (x86, x64, SPARCv9)
* MIPSpro (MIPS32)

======= Building =======

Use CMake to build plibsys for any target platform.

======= License =======

plibsys is distributed under the terms of GNU LGPLv2 or higher license.

======= More =======

More information about the library is available on the [Wiki](https://github.com/saprykin/plibsys/wiki).
