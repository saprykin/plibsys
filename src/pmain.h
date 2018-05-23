/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file pmain.h
 * @brief Library initialization
 * @author Alexander Saprykin
 *
 * Before using the library you must to initialize it properly. Use
 * p_libsys_init() to initialize the library. Please note that you need to call
 * it only once, not in every thread. This call is not MT-safe (because it also
 * initializes the threading subsystem itself), so it is best to place it in the
 * program's main thread, when the program starts.
 *
 * The only difference between p_libsys_init() and p_libsys_init_full() is that
 * the latter one allows to setup memory management routines before doing any
 * internal library call. This way you can ensure to use provided memory
 * management everywhere (even for library initialization).
 *
 * When you do not need the library anymore release used resourses with the
 * p_libsys_shutdown() routine. You should only call it once, too. This call is
 * not MT-safe (because it also deinitializes the threading subsystem itself),
 * so it is best to place it in the program's main thread, when the program
 * finishes.
 *
 * It is not recommended to call the initialization and deinitialization
 * routines on Windows in the DllMain() call because it may require libraries
 * other than kernel32.dll.
 */

/**
 * @mainpage
 * Basic
 * - @link
 * pmain.h Library initialization
 * @endlink
 * - @link
 * ptypes.h Data types
 * @endlink
 * - @link
 * pmacroscpu.h CPU detection macros
 * @endlink
 * - @link
 * pmacrosos.h OS detection macros
 * @endlink
 * - @link
 * pmacroscompiler.h Compiler detection macros
 * @endlink
 * - @link
 * pmacros.h Miscellaneous macros
 * @endlink
 * - @link
 * pstring.h Strings
 * @endlink
 *
 * System
 * - @link
 * pmem.h Memory management
 * @endlink
 * - @link
 * pprocess.h Process
 * @endlink
 * - @link
 * plibraryloader.h Shared library loader
 * @endlink
 * - @link
 * ptimeprofiler.h Time profiler
 * @endlink
 * - @link
 * perror.h Errors
 * @endlink
 *
 * Data structures
 * - @link
 * plist.h Singly linked list
 * @endlink
 * - @link
 * phashtable.h Hash table
 * @endlink
 * - @link
 * pcryptohash.h Cryptographic hash
 * @endlink
 * - @link
 * ptree.h Binary search tree
 * @endlink
 *
 * Multithreading
 * - @link
 * puthread.h Thread
 * @endlink
 * - @link
 * pmutex.h Mutex
 * @endlink
 * - @link
 * pcondvariable.h Condition variable
 * @endlink
 * - @link
 * prwlock.h Read-write lock
 * @endlink
 * - @link
 * pspinlock.h Spinlock
 * @endlink
 * - @link
 * patomic.h Atomic operations
 * @endlink
 *
 * Interprocess communication
 * - @link
 * psemaphore.h Semaphore
 * @endlink
 * - @link
 * pshm.h Shared memory
 * @endlink
 * - @link
 * pshmbuffer.h Shared memory buffer
 * @endlink
 *
 * Networking
 * - @link
 * psocketaddress.h Socket address
 * @endlink
 * - @link
 * psocket.h Socket
 * @endlink
 *
 * File and directories
 * - @link
 * pfile.h Files
 * @endlink
 * - @link
 * pdir.h Directories
 * @endlink
 * - @link
 * pinifile.h INI file parser
 * @endlink
 *
 * Stack
 * - @link
 * pstdarg.h Variable number of arguments
 * @endlink
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PMAIN_H
#define PLIBSYS_HEADER_PMAIN_H

#include <pmacros.h>
#include <pmem.h>

P_BEGIN_DECLS

/**
 * @brief Initializes library resources.
 * @since 0.0.1
 */
P_LIB_API void		p_libsys_init		(void);

/**
 * @brief Initializes library resources along with the memory table.
 * @param vtable Memory management table.
 * @since 0.0.1
 */
P_LIB_API void		p_libsys_init_full	(const PMemVTable *vtable);

/**
 * @brief Frees library resources. You should stop using any of the library
 * routines after calling this one.
 * @since 0.0.1
 */
P_LIB_API void		p_libsys_shutdown	(void);

/**
 * @brief Gets the library version against which it was compiled at run-time.
 * @return Library version.
 * @since 0.0.1
 * @note This version may differ from the version the application was compiled
 * against.
 * @sa #PLIBSYS_VERSION, #PLIBSYS_VERSION_STR, #PLIBSYS_VERSION_CHECK
 */
P_LIB_API const pchar *	p_libsys_version	(void);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PMAIN_H */
