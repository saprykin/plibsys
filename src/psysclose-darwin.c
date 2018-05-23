/*
 * The MIT License
 *
 * Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

/*
 * Copyright 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * http://crbug.com/269623
 * http://openradar.appspot.com/14999594
 *
 * When the default version of close used on macOS fails with EINTR, the
 * file descriptor is not in a deterministic state. It may have been closed,
 * or it may not have been. This makes it impossible to gracefully recover
 * from the error. If the close is retried after the FD has been closed, the
 * subsequent close can report EBADF, or worse, it can close an unrelated FD
 * opened by another thread. If the close is not retried after the FD has been
 * left open, the FD is leaked. Neither of these are good options.
 *
 * macOS provides an alternate version of close, close$NOCANCEL. This
 * version will never fail with EINTR before the FD is actually closed. With
 * this version, it is thus safe to call close without checking for EINTR (as
 * the HANDLE_EINTR macro does) and not risk leaking the FD. In fact, mixing
 * this verison of close with HANDLE_EINTR is hazardous.
 *
 * The $NOCANCEL variants of various system calls are activated by compiling
 * with __DARWIN_NON_CANCELABLE, which prevents them from being pthread
 * cancellation points. Rather than taking such a heavy-handed approach, this
 * file implements an alternative: to use the $NOCANCEL variant of close (thus
 * preventing it from being a pthread cancellation point) without affecting
 * any other system calls.
 *
 * This file operates by providing a close function with the non-$NOCANCEL
 * symbol name expected for the compilation environment as set by <unistd.h>
 * and <sys/cdefs.h> (the DARWIN_ALIAS_C macro). That function calls the
 * $NOCANCEL variant, which is resolved from libsyscall. By linking with this
 * version of close prior to the libsyscall version, close's implementation is
 * overridden.
 */

#include <sys/cdefs.h>

/* If the non-cancelable variants of all system calls have already been chosen,
 * do nothing. */
#if !__DARWIN_NON_CANCELABLE
#  if __DARWIN_UNIX03 && !__DARWIN_ONLY_UNIX_CONFORMANCE
/* When there's a choice between UNIX2003 and pre-UNIX2003 and UNIX2003 has
 * been chosen. */
extern int close$NOCANCEL$UNIX2003 (int fd);
#    define PLIBSYS_CLOSE_INTERFACE close$NOCANCEL$UNIX2003
#  elif !__DARWIN_UNIX03 && !__DARWIN_ONLY_UNIX_CONFORMANCE
/* When there's a choice between UNIX2003 and pre-UNIX2003 and pre-UNIX2003
 * has been chosen. There's no close$NOCANCEL symbol in this case, so use
 * close$NOCANCEL$UNIX2003 as the implementation. It does the same thing that
 * close$NOCANCEL would do. */
extern int close$NOCANCEL$UNIX2003 (int fd);
#    define PLIBSYS_CLOSE_INTERFACE close$NOCANCEL$UNIX2003
#  else
/* When only UNIX2003 is supported. */
extern int close$NOCANCEL (int fd);
#    define PLIBSYS_CLOSE_INTERFACE close$NOCANCEL
#  endif
#endif

#include "psysclose-private.h"

pint
p_sys_close (pint fd)
{
	return PLIBSYS_CLOSE_INTERFACE (fd);
}
