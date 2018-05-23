/*
 * The MIT License
 *
 * Copyright (C) 2016-2018 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "perror.h"
#include "pmem.h"
#include "pstring.h"
#include "perror-private.h"

#ifndef P_OS_WIN
#  if defined (P_OS_OS2)
#    define INCL_DOSERRORS
#    include <os2.h>
#    include <sys/socket.h>
#  endif
#  include <errno.h>
#endif

struct PError_ {
	pint	code;
	pint	native_code;
	pchar	*message;
};

PErrorIO
p_error_get_io_from_system (pint err_code)
{
	switch (err_code) {
	case 0:
		return P_ERROR_IO_NONE;
#if defined (P_OS_WIN)
#  ifdef WSAEADDRINUSE
	case WSAEADDRINUSE:
		return P_ERROR_IO_ADDRESS_IN_USE;
#  endif
#  ifdef WSAEWOULDBLOCK
	case WSAEWOULDBLOCK:
		return P_ERROR_IO_WOULD_BLOCK;
#  endif
#  ifdef WSAEACCES
	case WSAEACCES:
		return P_ERROR_IO_ACCESS_DENIED;
#  endif
#  ifdef WSA_INVALID_HANDLE
	case WSA_INVALID_HANDLE:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef WSA_INVALID_PARAMETER
	case WSA_INVALID_PARAMETER:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef WSAEBADF
	case WSAEBADF:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef WSAENOTSOCK
	case WSAENOTSOCK:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef WSAEINVAL
	case WSAEINVAL:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef WSAESOCKTNOSUPPORT
	case WSAESOCKTNOSUPPORT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#  ifdef WSAEOPNOTSUPP
	case WSAEOPNOTSUPP:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#  ifdef WSAEPFNOSUPPORT
	case WSAEPFNOSUPPORT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#  ifdef WSAEAFNOSUPPORT
	case WSAEAFNOSUPPORT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#  ifdef WSAEPROTONOSUPPORT
	case WSAEPROTONOSUPPORT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#  ifdef WSAECANCELLED
	case WSAECANCELLED:
		return P_ERROR_IO_ABORTED;
#  endif
#  ifdef ERROR_ALREADY_EXISTS
	case ERROR_ALREADY_EXISTS:
		return P_ERROR_IO_EXISTS;
#  endif
#  ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
		return P_ERROR_IO_NOT_EXISTS;
#  endif
#  ifdef ERROR_NO_MORE_FILES
	case ERROR_NO_MORE_FILES:
		return P_ERROR_IO_NO_MORE;
#  endif
#  ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
		return P_ERROR_IO_ACCESS_DENIED;
#  endif
#  ifdef ERROR_OUTOFMEMORY
	case ERROR_OUTOFMEMORY:
		return P_ERROR_IO_NO_RESOURCES;
#  endif
#  ifdef ERROR_NOT_ENOUGH_MEMORY
	case ERROR_NOT_ENOUGH_MEMORY:
		return P_ERROR_IO_NO_RESOURCES;
#  endif
#  ifdef ERROR_INVALID_HANDLE
#    if !defined(WSA_INVALID_HANDLE) || (ERROR_INVALID_HANDLE != WSA_INVALID_HANDLE)
	case ERROR_INVALID_HANDLE:
		return P_ERROR_IO_INVALID_ARGUMENT;
#    endif
#  endif
#  ifdef ERROR_INVALID_PARAMETER
#    if !defined(WSA_INVALID_PARAMETER) || (ERROR_INVALID_PARAMETER != WSA_INVALID_PARAMETER)
	case ERROR_INVALID_PARAMETER:
		return P_ERROR_IO_INVALID_ARGUMENT;
#    endif
#  endif
#  ifdef ERROR_NOT_SUPPORTED
	case ERROR_NOT_SUPPORTED:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#elif defined (P_OS_OS2)
#  ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
		return P_ERROR_IO_NOT_EXISTS;
#  endif
#  ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
		return P_ERROR_IO_NOT_EXISTS;
#  endif
#  ifdef ERROR_TOO_MANY_OPEN_FILES
	case ERROR_TOO_MANY_OPEN_FILES:
		return P_ERROR_IO_NO_RESOURCES;
#  endif
#  ifdef ERROR_NOT_ENOUGH_MEMORY
	case ERROR_NOT_ENOUGH_MEMORY:
		return P_ERROR_IO_NO_RESOURCES;
#  endif
#  ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
		return P_ERROR_IO_ACCESS_DENIED;
#  endif
#  ifdef ERROR_INVALID_HANDLE
	case ERROR_INVALID_HANDLE:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_INVALID_PARAMETER
	case ERROR_INVALID_PARAMETER:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_INVALID_ADDRESS
	case ERROR_INVALID_ADDRESS:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_NO_MORE_FILES
	case ERROR_NO_MORE_FILES:
		return P_ERROR_IO_NO_MORE;
#  endif
#  ifdef ERROR_NOT_SUPPORTED
	case ERROR_NOT_SUPPORTED:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif
#  ifdef ERROR_FILE_EXISTS
	case ERROR_FILE_EXISTS:
		return P_ERROR_IO_EXISTS;
#  endif
#else /* !P_OS_WIN && !P_OS_OS2 */
#  ifdef EACCES
	case EACCES:
		return P_ERROR_IO_ACCESS_DENIED;
#  endif

#  ifdef EPERM
	case EPERM:
		return P_ERROR_IO_ACCESS_DENIED;
#  endif

#  ifdef ENOMEM
	case ENOMEM:
		return P_ERROR_IO_NO_RESOURCES;
#  endif

#  ifdef ENOSR
	case ENOSR:
		return P_ERROR_IO_NO_RESOURCES;
#  endif

#  ifdef ENOBUFS
	case ENOBUFS:
		return P_ERROR_IO_NO_RESOURCES;
#  endif

#  ifdef ENFILE
	case ENFILE:
		return P_ERROR_IO_NO_RESOURCES;
#  endif

#  ifdef ENOSPC
	case ENOSPC:
		return P_ERROR_IO_NO_RESOURCES;
#  endif

#  ifdef EMFILE
	case EMFILE:
		return P_ERROR_IO_NO_RESOURCES;
#  endif

#  ifdef EINVAL
	case EINVAL:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif

#  ifdef EBADF
	case EBADF:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif

#  ifdef ENOTSOCK
	case ENOTSOCK:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif

#  ifdef EFAULT
	case EFAULT:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif

#  ifdef EPROTOTYPE
	case EPROTOTYPE:
		return P_ERROR_IO_INVALID_ARGUMENT;
#  endif

	/* On Linux these errors can have same codes */
#  if defined(ENOTSUP) && (!defined(EOPNOTSUPP) || ENOTSUP != EOPNOTSUPP)
	case ENOTSUP:
			return P_ERROR_IO_NOT_SUPPORTED;
#  endif

#  ifdef ENOPROTOOPT
	case ENOPROTOOPT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif

#  ifdef EPROTONOSUPPORT
	case EPROTONOSUPPORT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif

#  ifdef EAFNOSUPPORT
	case EAFNOSUPPORT:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif

#  ifdef EOPNOTSUPP
	case EOPNOTSUPP:
		return P_ERROR_IO_NOT_SUPPORTED;
#  endif

#  ifdef EADDRNOTAVAIL
	case EADDRNOTAVAIL:
		return P_ERROR_IO_NOT_AVAILABLE;
#  endif

#  ifdef ENETUNREACH
	case ENETUNREACH:
		return P_ERROR_IO_NOT_AVAILABLE;
#  endif

#  ifdef ENETDOWN
	case ENETDOWN:
		return P_ERROR_IO_NOT_AVAILABLE;
#  endif

#  ifdef EHOSTDOWN
	case EHOSTDOWN:
		return P_ERROR_IO_NOT_AVAILABLE;
#  endif

#  ifdef ENONET
	case ENONET:
		return P_ERROR_IO_NOT_AVAILABLE;
#  endif

#  ifdef EHOSTUNREACH
	case EHOSTUNREACH:
		return P_ERROR_IO_NOT_AVAILABLE;
#  endif

#  ifdef EINPROGRESS
	case EINPROGRESS:
		return P_ERROR_IO_IN_PROGRESS;
#  endif

#  ifdef EALREADY
	case EALREADY:
		return P_ERROR_IO_IN_PROGRESS;
#  endif

#  ifdef EISCONN
	case EISCONN:
		return P_ERROR_IO_CONNECTED;
#  endif

#  ifdef ECONNREFUSED
	case ECONNREFUSED:
		return P_ERROR_IO_CONNECTION_REFUSED;
#  endif

#  ifdef ENOTCONN
	case ENOTCONN:
		return P_ERROR_IO_NOT_CONNECTED;
#  endif

#  ifdef ECONNABORTED
	case ECONNABORTED:
		return P_ERROR_IO_ABORTED;
#  endif

#  ifdef EADDRINUSE
	case EADDRINUSE:
		return P_ERROR_IO_ADDRESS_IN_USE;
#  endif

#  ifdef ETIMEDOUT
	case ETIMEDOUT:
		return P_ERROR_IO_TIMED_OUT;
#  endif

#  ifdef EDQUOT
	case EDQUOT:
		return P_ERROR_IO_QUOTA;
#  endif

#  ifdef EISDIR
	case EISDIR:
		return P_ERROR_IO_IS_DIRECTORY;
#  endif

#  ifdef ENOTDIR
	case ENOTDIR:
		return P_ERROR_IO_NOT_DIRECTORY;
#  endif

#  ifdef EEXIST
	case EEXIST:
		return P_ERROR_IO_EXISTS;
#  endif

#  ifdef ENOENT
	case ENOENT:
		return P_ERROR_IO_NOT_EXISTS;
#  endif

#  ifdef ENAMETOOLONG
	case ENAMETOOLONG:
		return P_ERROR_IO_NAMETOOLONG;
#  endif

#  ifdef ENOSYS
	case ENOSYS:
		return P_ERROR_IO_NOT_IMPLEMENTED;
#  endif

	/* Some magic to deal with EWOULDBLOCK and EAGAIN.
	 * Apparently on HP-UX these are actually defined to different values,
	 * but on Linux, for example, they are the same. */
#  if defined(EWOULDBLOCK) && defined(EAGAIN) && EWOULDBLOCK == EAGAIN
	/* We have both and they are the same: only emit one case. */
	case EAGAIN:
		return P_ERROR_IO_WOULD_BLOCK;
#  else
	/* Else: consider each of them separately. This handles both the
	 * case of having only one and the case where they are different values. */
#    ifdef EAGAIN
	case EAGAIN:
		return P_ERROR_IO_WOULD_BLOCK;
#    endif

#    ifdef EWOULDBLOCK
	case EWOULDBLOCK:
		return P_ERROR_IO_WOULD_BLOCK;
#    endif
#  endif
#endif /* !P_OS_WIN */
	default:
		return P_ERROR_IO_FAILED;
	}
}

PErrorIO
p_error_get_last_io (void)
{
	return p_error_get_io_from_system (p_error_get_last_system ());
}

PErrorIPC
p_error_get_ipc_from_system (pint err_code)
{
	switch (err_code) {
	case 0:
		return P_ERROR_IPC_NONE;
#ifdef P_OS_WIN
#  ifdef ERROR_ALREADY_EXISTS
	case ERROR_ALREADY_EXISTS:
		return P_ERROR_IPC_EXISTS;
#  endif
#  ifdef ERROR_SEM_OWNER_DIED
	case ERROR_SEM_OWNER_DIED:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif
#  ifdef ERROR_SEM_NOT_FOUND
	case ERROR_SEM_NOT_FOUND:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif
#  ifdef ERROR_SEM_USER_LIMIT
	case ERROR_SEM_USER_LIMIT:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_TOO_MANY_SEMAPHORES
	case ERROR_TOO_MANY_SEMAPHORES:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
		return P_ERROR_IPC_ACCESS;
#  endif
#  ifdef ERROR_EXCL_SEM_ALREADY_OWNED
	case ERROR_EXCL_SEM_ALREADY_OWNED:
		return P_ERROR_IPC_ACCESS;
#  endif
#  ifdef ERROR_TOO_MANY_SEM_REQUESTS
	case ERROR_TOO_MANY_SEM_REQUESTS:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_TOO_MANY_POSTS
	case ERROR_TOO_MANY_POSTS:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_OUTOFMEMORY
	case ERROR_OUTOFMEMORY:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_NOT_ENOUGH_MEMORY
	case ERROR_NOT_ENOUGH_MEMORY:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_INVALID_HANDLE
	case ERROR_INVALID_HANDLE:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_INVALID_PARAMETER
	case ERROR_INVALID_PARAMETER:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_NOT_SUPPORTED
	case ERROR_NOT_SUPPORTED:
		return P_ERROR_IPC_NOT_IMPLEMENTED;
#  endif
#elif defined (P_OS_OS2)
#  ifdef ERROR_NOT_ENOUGH_MEMORY
	case ERROR_NOT_ENOUGH_MEMORY:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_INVALID_PARAMETER
	case ERROR_INVALID_PARAMETER:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_INVALID_NAME
	case ERROR_INVALID_NAME:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_INVALID_HANDLE
	case ERROR_INVALID_HANDLE:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif
#  ifdef ERROR_INVALID_ADDRESS
	case ERROR_INVALID_ADDRESS:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif
#  ifdef ERROR_GEN_FAILURE
	case ERROR_GEN_FAILURE:
		return P_ERROR_IPC_FAILED;
#  endif
#  ifdef ERROR_LOCKED
	case ERROR_LOCKED:
		return P_ERROR_IPC_ACCESS;
#  endif
#  ifdef ERROR_DUPLICATE_NAME
	case ERROR_DUPLICATE_NAME:
		return P_ERROR_IPC_EXISTS;
#  endif
#  ifdef ERROR_TOO_MANY_HANDLES
	case ERROR_TOO_MANY_HANDLES:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_TOO_MANY_OPENS
	case ERROR_TOO_MANY_OPENS:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_TOO_MANY_SEM_REQUESTS
	case ERROR_TOO_MANY_SEM_REQUESTS:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif
#  ifdef ERROR_SEM_OWNER_DIED
	case ERROR_SEM_OWNER_DIED:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif
#  ifdef ERROR_NOT_OWNER
	case ERROR_NOT_OWNER:
		return P_ERROR_IPC_ACCESS;
#  endif
#  ifdef ERROR_SEM_NOT_FOUND
	case ERROR_SEM_NOT_FOUND:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif
#else /* !P_OS_WINDOWS && !P_OS_OS2 */
#  ifdef EACCES
	case EACCES:
		return P_ERROR_IPC_ACCESS;
#  endif

#  ifdef EPERM
	case EPERM:
		return P_ERROR_IPC_ACCESS;
#  endif

#  ifdef EEXIST
	case EEXIST:
		return P_ERROR_IPC_EXISTS;
#  endif

#  ifdef E2BIG
	case E2BIG:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif

#  ifdef EFAULT
	case EFAULT:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif

#  ifdef EFBIG
	case EFBIG:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif

#  ifdef EINVAL
	case EINVAL:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif

#  ifdef ELOOP
	case ELOOP:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif

#  ifdef ERANGE
	case ERANGE:
		return P_ERROR_IPC_INVALID_ARGUMENT;
#  endif

#  ifdef ENOMEM
	case ENOMEM:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif

#  ifdef EMFILE
	case EMFILE:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif

#  ifdef ENFILE
	case ENFILE:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif

#  ifdef ENOSPC
	case ENOSPC:
		return P_ERROR_IPC_NO_RESOURCES;
#  endif

#  ifdef EIDRM
	case EIDRM:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif

#  ifdef ENOENT
	case ENOENT:
		return P_ERROR_IPC_NOT_EXISTS;
#  endif

#  ifdef EOVERFLOW
	case EOVERFLOW:
		return P_ERROR_IPC_OVERFLOW;
#  endif

#  ifdef ENOSYS
	case ENOSYS:
		return P_ERROR_IPC_NOT_IMPLEMENTED;
#  endif

#  ifdef EDEADLK
	case EDEADLK:
		return P_ERROR_IPC_DEADLOCK;
#  endif

#  ifdef ENAMETOOLONG
	case ENAMETOOLONG:
		return P_ERROR_IPC_NAMETOOLONG;
#  endif
#endif /* !P_OS_WIN */
	default:
		return P_ERROR_IPC_FAILED;
	}
}

PErrorIPC
p_error_get_last_ipc (void)
{
	return p_error_get_ipc_from_system (p_error_get_last_system ());
}

P_LIB_API PError *
p_error_new (void)
{
	PError *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PError))) == NULL))
		return NULL;

	return ret;
}

P_LIB_API PError *
p_error_new_literal (pint		code,
		     pint		native_code,
		     const pchar	*message)
{
	PError *ret;

	if (P_UNLIKELY ((ret = p_error_new ()) == NULL))
		return NULL;

	ret->code        = code;
	ret->native_code = native_code;
	ret->message     = p_strdup (message);

	return ret;
}

P_LIB_API const pchar *
p_error_get_message (PError *error)
{
	if (P_UNLIKELY (error == NULL))
		return NULL;

	return error->message;
}

P_LIB_API pint
p_error_get_code (PError *error)
{
	if (P_UNLIKELY (error == NULL))
		return 0;

	return error->code;
}

P_LIB_API pint
p_error_get_native_code	(PError	*error)
{
	if (P_UNLIKELY (error == NULL))
		return 0;

	return error->native_code;
}

P_LIB_API PErrorDomain
p_error_get_domain (PError *error)
{
	if (P_UNLIKELY (error == NULL))
		return P_ERROR_DOMAIN_NONE;

	if (error->code >= (pint) P_ERROR_DOMAIN_IPC)
		return P_ERROR_DOMAIN_IPC;
	else if (error->code >= (pint) P_ERROR_DOMAIN_IO)
		return P_ERROR_DOMAIN_IO;
	else
		return P_ERROR_DOMAIN_NONE;
}

P_LIB_API PError *
p_error_copy (PError *error)
{
	PError *ret;

	if (P_UNLIKELY (error == NULL))
		return NULL;

	if (P_UNLIKELY ((ret = p_error_new_literal (error->code,
						    error->native_code,
						    error->message)) == NULL))
		return NULL;

	return ret;
}

P_LIB_API void
p_error_set_error (PError	*error,
		   pint		code,
		   pint		native_code,
		   const pchar	*message)
{
	if (P_UNLIKELY (error == NULL))
		return;

	if (error->message != NULL)
		p_free (error->message);

	error->code        = code;
	error->native_code = native_code;
	error->message     = p_strdup (message);
}

P_LIB_API void
p_error_set_error_p (PError		**error,
		     pint		code,
		     pint		native_code,
		     const pchar	*message)
{
	if (error == NULL || *error != NULL)
		return;

	*error = p_error_new_literal (code, native_code, message);
}

P_LIB_API void
p_error_set_code (PError	*error,
		  pint		code)
{
	if (P_UNLIKELY (error == NULL))
		return;

	error->code = code;
}

P_LIB_API void
p_error_set_native_code	(PError	*error,
			 pint	native_code)
{
	if (P_UNLIKELY (error == NULL))
		return;

	error->native_code = native_code;
}

P_LIB_API void
p_error_set_message (PError		*error,
		     const pchar	*message)
{
	if (P_UNLIKELY (error == NULL))
		return;

	if (error->message != NULL)
		p_free (error->message);

	error->message = p_strdup (message);
}

P_LIB_API void
p_error_clear (PError *error)
{
	if (P_UNLIKELY (error == NULL))
		return;

	if (error->message != NULL)
		p_free (error->message);

	error->message     = NULL;
	error->code        = 0;
	error->native_code = 0;
}

P_LIB_API void
p_error_free (PError	*error)
{
	if (P_UNLIKELY (error == NULL))
		return;

	if (error->message != NULL)
		p_free (error->message);

	p_free (error);
}

P_LIB_API pint
p_error_get_last_system (void)
{
#ifdef P_OS_WIN
	return (pint) GetLastError ();
#else
#  ifdef P_OS_VMS
	pint error_code = errno;

	if (error_code == EVMSERR)
		return vaxc$errno;
	else
		return error_code;
#  else
	return errno;
#  endif
#endif
}

P_LIB_API pint
p_error_get_last_net (void)
{
#if defined (P_OS_WIN)
	return WSAGetLastError ();
#elif defined (P_OS_OS2)
	return sock_errno ();
#else
	return p_error_get_last_system ();
#endif
}

P_LIB_API void
p_error_set_last_system (pint code)
{
#ifdef P_OS_WIN
	SetLastError (code);
#else
	errno = code;
#endif
}

P_LIB_API void
p_error_set_last_net (pint code)
{
#if defined (P_OS_WIN)
	WSASetLastError (code);
#elif defined (P_OS_OS2)
	P_UNUSED (code);
#else
	errno = code;
#endif
}
