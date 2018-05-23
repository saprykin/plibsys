/*
 * The MIT License
 *
 * Copyright (C) 2017-2018 Alexander Saprykin <saprykin.spb@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file pmacrosos.h
 * @brief OS detection macros
 * @author Alexander Saprykin
 *
 * All the macros are completely independent of any other platform-specific
 * headers, thus gurantee to work with any compiler under any operating system
 * in the same way as they are used within the library.
 *
 * This family of macros provides OS detection and defines one or several of
 * P_OS_x macros.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PMACROSOS_H
#define PLIBSYS_HEADER_PMACROSOS_H

/*
 * List of supported operating systems (P_OS_x):
 *
 * DARWIN            - Any Darwin based system
 * DARWIN32          - Any 32-bit Darwin based system
 * DARWIN64          - Any 64-bit Darwin based system
 * BSD4              - Any BSD 4.x based system
 * FREEBSD           - FreeBSD
 * DRAGONFLY         - DragonFlyBSD
 * NETBSD            - NetBSD
 * OPENBSD           - OpenBSD
 * AIX               - IBM AIX
 * HPUX              - HP-UX
 * TRU64             - Tru64
 * SOLARIS           - Sun (Oracle) Solaris
 * QNX               - QNX 4.x
 * QNX6              - QNX Neutrino 6.x
 * BB10              - BlackBerry 10
 * SCO               - SCO OpenServer 5/6
 * UNIXWARE          - UnixWare 7
 * IRIX              - SGI IRIX
 * HAIKU             - Haiku
 * SYLLABLE          - Syllable
 * BEOS              - BeOS
 * OS2               - OS/2
 * VMS               - OpenVMS
 * AMIGA             - AmigaOS
 * UNIX              - Any UNIX BSD/SYSV based system
 * LINUX             - Linux
 * MAC9              - Mac OS 9 (Classic)
 * MAC               - Any macOS
 * MAC32             - 32-bit macOS
 * MAC64             - 64-bit macOS
 * CYGWIN            - Cygwin
 * MSYS              - MSYS
 * WIN               - 32-bit Windows
 * WIN64             - 64-bit Windows
 * ANDROID           - Android
 */

/**
 * @def P_OS_DARWIN
 * @brief Darwin based operating system (i.e. macOS).
 * @since 0.0.1
 */

/**
 * @def P_OS_DARWIN32
 * @brief Darwin based 32-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_DARWIN64
 * @brief Darwin based 64-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_BSD4
 * @brief BSD 4.x based operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_FREEBSD
 * @brief FreeBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_DRAGONFLY
 * @brief DragonFlyBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_NETBSD
 * @brief NetBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_OPENBSD
 * @brief OpenBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_AIX
 * @brief IBM AIX operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_HPUX
 * @brief HP-UX operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_TRU64
 * @brief Tru64 operating system.
 * @since 0.0.2
 */

/**
 * @def P_OS_SOLARIS
 * @brief Sun (Oracle) Solaris operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_QNX
 * @brief QNX 4.x operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_QNX6
 * @brief QNX Neutrino 6.x operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_BB10
 * @brief BlackBerry 10 operating system.
 * @since 0.0.4
 */

/**
 * @def P_OS_SCO
 * @brief SCO OpenServer operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_UNIXWARE
 * @brief UnixWare 7 operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_IRIX
 * @brief SGI's IRIX operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_HAIKU
 * @brief Haiku operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_SYLLABLE
 * @brief Syllable operating system.
 * @since 0.0.2
 */

/**
 * @def P_OS_BEOS
 * @brief BeOS operating system.
 * @since 0.0.3
 */

/**
 * @def P_OS_OS2
 * @brief OS/2 operating system.
 * @since 0.0.3
 */

/**
 * @def P_OS_VMS
 * @brief OpenVMS operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_AMIGA
 * @brief AmigaOS operating system.
 * @since 0.0.4
 */

/**
 * @def P_OS_UNIX
 * @brief UNIX based operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_LINUX
 * @brief Linux based operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC9
 * @brief Apple's Mac OS 9 operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC
 * @brief Apple's macOS operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC32
 * @brief Apple's macOS 32-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC64
 * @brief Apple's macOS 64-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_CYGWIN
 * @brief Microsoft Windows POSIX runtime environment.
 * @since 0.0.1
 */

/**
 * @def P_OS_MSYS
 * @brief Microsoft Windows POSIX development environment.
 * @since 0.0.1
 */

/**
 * @def P_OS_WIN
 * @brief Microsoft Windows 32-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_WIN64
 * @brief Microsoft Windows 64-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_ANDROID
 * @brief Google Android
 * @since 0.0.4
 */

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define P_OS_DARWIN
#  define P_OS_BSD4
#  ifdef __LP64__
#    define P_OS_DARWIN64
#  else
#    define P_OS_DARWIN32
#  endif
# elif defined(Macintosh) || defined(macintosh)
#  define P_OS_MAC9
#elif defined(__MSYS__)
#  define P_OS_MSYS
#elif defined(__CYGWIN__)
#  define P_OS_CYGWIN
#elif defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64)
#  define P_OS_WIN64
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#  define P_OS_WIN
#elif defined(__ANDROID__)
#  define P_OS_ANDROID
#  define P_OS_LINUX
#elif defined(__linux) || defined(__linux__)
#  define P_OS_LINUX
#elif defined(__FreeBSD__)
#  define P_OS_FREEBSD
#  define P_OS_BSD4
#elif defined(__DragonFly__)
#  define P_OS_DRAGONFLY
#  define P_OS_BSD4
#elif defined(__NetBSD__)
#  define P_OS_NETBSD
#  define P_OS_BSD4
#elif defined(__OpenBSD__)
#  define P_OS_OPENBSD
#  define P_OS_BSD4
#elif defined(_AIX)
#  define P_OS_AIX
#elif defined(hpux) || defined(__hpux)
#  define P_OS_HPUX
#elif defined(__osf__) || defined(__osf)
#  define P_OS_TRU64
#elif defined(__sun) || defined(sun)
#  define P_OS_SOLARIS
#elif defined(__QNXNTO__)
#  ifdef __BLACKBERRY10__
#    define P_OS_BB10
#  else
#    define P_OS_QNX6
#  endif
#elif defined(__QNX__)
#  define P_OS_QNX
#elif defined(_SCO_DS)
#  define P_OS_SCO
#elif defined(__USLC__) || defined(__UNIXWARE__)
#  define P_OS_UNIXWARE
#elif defined(__svr4__) && defined(i386)
#  define P_OS_UNIXWARE
#elif defined(__sgi) || defined(sgi)
#  define P_OS_IRIX
#elif defined(__HAIKU__)
#  define P_OS_HAIKU
#elif defined(__SYLLABLE__)
#  define P_OS_SYLLABLE
#elif defined(__BEOS__)
#  define P_OS_BEOS
#elif defined(__OS2__)
#  define P_OS_OS2
#elif defined(VMS) || defined(__VMS)
#  define P_OS_VMS
#elif defined(AMIGA) || defined(__amigaos__)
#  define P_OS_AMIGA
#endif

#ifdef P_OS_WIN64
#  define P_OS_WIN
#endif

#if defined(P_OS_DARWIN)
#  define P_OS_MAC
#  if defined(P_OS_DARWIN64)
#     define P_OS_MAC64
#  elif defined(P_OS_DARWIN32)
#     define P_OS_MAC32
#  endif
#endif

#if defined(P_OS_WIN)  || defined(P_OS_MAC9) || defined(P_OS_HAIKU) || \
    defined(P_OS_BEOS) || defined(P_OS_OS2)  || defined(P_OS_VMS)   || \
    defined(P_OS_AMIGA)
#  undef P_OS_UNIX
#elif !defined(P_OS_UNIX)
#  define P_OS_UNIX
#endif

/* We need this to generate full Doxygen documentation */

#ifdef DOXYGEN
#  ifndef P_OS_DARWIN
#    define P_OS_DARWIN
#  endif
#  ifndef P_OS_DARWIN32
#    define P_OS_DARWIN32
#  endif
#  ifndef P_OS_DARWIN64
#    define P_OS_DARWIN64
#  endif
#  ifndef P_OS_BSD4
#    define P_OS_BSD4
#  endif
#  ifndef P_OS_FREEBSD
#    define P_OS_FREEBSD
#  endif
#  ifndef P_OS_DRAGONFLY
#    define P_OS_DRAGONFLY
#  endif
#  ifndef P_OS_NETBSD
#    define P_OS_NETBSD
#  endif
#  ifndef P_OS_OPENBSD
#    define P_OS_OPENBSD
#  endif
#  ifndef P_OS_AIX
#    define P_OS_AIX
#  endif
#  ifndef P_OS_HPUX
#    define P_OS_HPUX
#  endif
#  ifndef P_OS_TRU64
#    define P_OS_TRU64
#  endif
#  ifndef P_OS_SOLARIS
#    define P_OS_SOLARIS
#  endif
#  ifndef P_OS_QNX
#    define P_OS_QNX
#  endif
#  ifndef P_OS_QNX6
#    define P_OS_QNX6
#  endif
#  ifndef P_OS_BB10
#    define P_OS_BB10
#  endif
#  ifndef P_OS_SCO
#    define P_OS_SCO
#  endif
#  ifndef P_OS_UNIXWARE
#    define P_OS_UNIXWARE
#  endif
#  ifndef P_OS_IRIX
#    define P_OS_IRIX
#  endif
#  ifndef P_OS_HAIKU
#    define P_OS_HAIKU
#  endif
#  ifndef P_OS_SYLLABLE
#    define P_OS_SYLLABLE
#  endif
#  ifndef P_OS_BEOS
#    define P_OS_BEOS
#  endif
#  ifndef P_OS_OS2
#    define P_OS_OS2
#  endif
#  ifndef P_OS_VMS
#    define P_OS_VMS
#  endif
#  ifndef P_OS_AMIGA
#    define P_OS_AMIGA
#  endif
#  ifndef P_OS_UNIX
#    define P_OS_UNIX
#  endif
#  ifndef P_OS_LINUX
#    define P_OS_LINUX
#  endif
#  ifndef P_OS_MAC9
#    define P_OS_MAC9
#  endif
#  ifndef P_OS_MAC
#    define P_OS_MAC
#  endif
#  ifndef P_OS_MAC32
#    define P_OS_MAC32
#  endif
#  ifndef P_OS_MAC64
#    define P_OS_MAC64
#  endif
#  ifndef P_OS_CYGWIN
#    define P_OS_CYGWIN
#  endif
#  ifndef P_OS_MSYS
#    define P_OS_MSYS
#  endif
#  ifndef P_OS_WIN
#    define P_OS_WIN
#  endif
#  ifndef P_OS_WIN64
#    define P_OS_WIN64
#  endif
#  ifndef P_OS_ANDROID
#    define P_OS_ANDROID
#  endif
#endif

#endif /* PLIBSYS_HEADER_PMACROSOS_H */
