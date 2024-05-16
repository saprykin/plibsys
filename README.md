[![](https://github.com/saprykin/plibsys/actions/workflows/actions.yml/badge.svg)](https://github.com/saprykin/plibsys/actions)
[![](https://ci.appveyor.com/api/projects/status/yvu21lawtkkfpn4b/branch/master?svg=true)](https://ci.appveyor.com/project/saprykin/plibsys/branch/master)
[![](https://sonarcloud.io/api/project_badges/measure?project=saprykin_plibsys&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=saprykin_plibsys)
[![](https://scan.coverity.com/projects/8333/badge.svg)](https://scan.coverity.com/projects/saprykin-plibsys)
[![](https://codecov.io/gh/saprykin/plibsys/branch/coverage/graph/badge.svg)](https://codecov.io/gh/saprykin/plibsys)
[![](https://codedocs.xyz/saprykin/plibsys.svg)](https://codedocs.xyz/saprykin/plibsys/)
[![](https://img.shields.io/badge/license-MIT-ff69b4.svg)](https://opensource.org/licenses/MIT)

## About

plibsys is a cross-platform system C library with some helpful routines.
It has zero third-party dependencies and uses only native system calls.

## Features

plibsys gives you:

* Platform independent data types
* Threads 
* Mutexes
* Condition variables
* Read-write locks
* System-wide semaphores
* System-wide shared memory
* Optimized spinlock
* Atomic operations
* Socket support (UDP, TCP, SCTP) with IPv4 and IPv6
* Hash functions: MD5, SHA-1, SHA-2, SHA-3, GOST (R 34.11-94)
* Binary trees: BST, red-black, AVL
* INI file parser
* High resolution time profiler
* Files and directories
* Shared library loading
* Useful routines for linked lists, strings, hash tables
* Macros for CPU architecture, OS and compiler detection
* Macros for variable arguments

To achieve maximum native performance on each platform several implementation models are used:

* Threading models: POSIX, Solaris, OS/2, BeOS, AtheOS, AmigaOS and Win32
* IPC models: POSIX, System V, OS/2, AmigaOS and Win32
* Time profiler models: POSIX, Solaris, Mach, OS/2, BeOS, AmigaOS, Win32 and generic
* Directory iterating models: POSIX, OS/2 and Win32
* Shared library loading models: POSIX, HP-UX, OS/2, BeOS, AmigaOS and Win32
* Atomic operations models: sync, C11, DECC, Win32 and simulated
* Sockets: BSD with Win32 support

## Platforms

plibsys is a cross-platform, highly portable library, it is fully covered
with unit tests and was tested on the following desktop platforms:

* GNU/Linux
* macOS
* Windows
* Cygwin, MSYS
* FreeBSD, NetBSD, OpenBSD
* DragonFlyBSD
* Solaris
* AIX
* HP-UX
* Tru64
* OpenVMS
* OS/2
* IRIX
* QNX Neutrino
* UnixWare 7
* SCO OpenServer 5
* Haiku
* Syllable
* BeOS
* AmigaOS

plibsys also supports the following mobile platforms:

* iOS, watchOS, tvOS
* Android
* BlackBerry 10

It should also work on other *nix systems with or without minimal efforts.

## Compilers

plibsys was tested with the following compilers:

* MSVC (x86, x64) 2003 and above
* MinGW (x86, x64)
* Open Watcom (x86)
* Borland (x86)
* GCC (x86, x64, PPC32be, PPC64be/le, IA-64/32, Alpha, HPPA2.0-32, MIPS64/32, AArch32/64, SPARCv8/9, S390x, LoongArch64, RISC-V)
* Clang (x86, x64, PPC32be, PPC64le, AArch32/64, S390x, LoongArch64, RISC-V)
* Intel (x86, x64)
* QCC (x86, AArch32)
* Oracle Solaris Studio (x86, x64, SPARCv8/9)
* MIPSpro (MIPS32)
* XL C (PPC64le)
* DEC C (Alpha, IA-64, x64)
* PGI (x86, x64)
* Cray (x64)

## Building

Use CMake to build plibsys for target platform. For OpenVMS see platforms/vms-general directory.

## Documentation

Documentation for the latest stable verison is avaialble through the [GitHub Pages](http://saprykin.github.io/plibsys-docs).

## License

plibsys is distributed under the terms of MIT license.

## More

More information about the library is available on the [Wiki](https://github.com/saprykin/plibsys/wiki).
