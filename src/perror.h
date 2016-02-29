/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

/**
 * @file perror.h
 * @brief An error report system
 * @author Alexander Saprykin
 *
 * An error report system is used to notify a caller about fatal situations
 * during the library API invocation. Usually the sequence is as following:
 * @code
 * PError *error = NULL;
 * ...
 *
 * if (error != NULL) {
 *         ...
 *         p_error_free (error);
 * }
 * @endcode
 * Note that you should not initialize a new #PError object before passing the
 * pointer into API call. Simply initialize it with zero and check result after.
 * Therefore you need to free memory if error occurred.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PERROR_H__
#define __PERROR_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Enum with error domains */
typedef enum _PErrorDomain {
	P_ERROR_DOMAIN_NONE		= 0,	/**< No domain was specified.				*/
	P_ERROR_DOMAIN_IO		= 500,	/**< Input/output domain.				*/
	P_ERROR_DOMAIN_IPC		= 600	/**< Interprocess communication domain.			*/
} PErrorDomain;

/** Enum with IO errors */
typedef enum _PErrorIO {
	P_ERROR_IO_NONE			= 500,	/**< No error.						*/
	P_ERROR_IO_NO_RESOURCES		= 501,	/**< OS hasn't enough resources.			*/
	P_ERROR_IO_NOT_AVAILABLE	= 502,	/**< Resource isn't available.				*/
	P_ERROR_IO_ACCESS_DENIED	= 503,	/**< Access denied.					*/
	P_ERROR_IO_CONNECTED		= 504,	/**< Already connected.					*/
	P_ERROR_IO_IN_PROGRESS		= 505,	/**< Operation in progress.				*/
	P_ERROR_IO_ABORTED		= 506,	/**< Operation aborted.					*/
	P_ERROR_IO_INVALID_ARGUMENT	= 507,	/**< Invalid argument specified.			*/
	P_ERROR_IO_NOT_SUPPORTED	= 508,	/**< Operation not supported.				*/
	P_ERROR_IO_TIMED_OUT		= 509,	/**< Operation timed out.				*/
	P_ERROR_IO_WOULD_BLOCK		= 510,	/**< Operation cannot be completed immediatly.		*/
	P_ERROR_IO_ADDRESS_IN_USE	= 511,	/**< Address is already under usage.			*/
	P_ERROR_IO_CONNECTION_REFUSED	= 512,	/**< Connection refused.				*/
	P_ERROR_IO_NOT_CONNECTED	= 513,	/**< Connection required first.				*/
	P_ERROR_IO_QUOTA		= 514,  /**< User quota exceeded.				*/
	P_ERROR_IO_IS_DIRECTORY		= 515,  /**< Trying to open directory for writting.		*/
	P_ERROR_IO_NOT_DIRECTORY	= 516,  /**< Component of the path prefix is not a directory.	*/
	P_ERROR_IO_NAMETOOLONG		= 517,  /**< Specified name is too long.			*/
	P_ERROR_IO_EXISTS		= 518,  /**< Specified already exists.				*/
	P_ERROR_IO_NOT_EXISTS		= 519,  /**< Specified entry doesn't exist.			*/
	P_ERROR_IO_FAILED		= 520	/**< General error.					*/
} PErrorIO;

/** Enum with IPC errors */
typedef enum _PErrorIPC {
	P_ERROR_IPC_NONE		= 600,	/**< No error.						*/
	P_ERROR_IPC_ACCESS		= 601,	/**< Not enough rights to access object or its key.	*/
	P_ERROR_IPC_EXISTS		= 602,	/**< Object already exists and no proper open flags
						     were specified.					*/
	P_ERROR_IPC_NOT_EXISTS		= 603,	/**< Object doesn't exist or was removed before, and
						     no proper create flags were specified.		*/
	P_ERROR_IPC_NO_RESOURCES	= 604,	/**< Not enough system resources or memory to perform
						     operation.						*/
	P_ERROR_IPC_OVERFLOW		= 605,	/**< Semaphore value overflow.				*/
	P_ERROR_IPC_NAMETOOLONG		= 606,	/**< Object name is too long.				*/
	P_ERROR_IPC_INVALID_ARGUMENT	= 607,	/**< Invalid argument (parameter) specified.		*/
	P_ERROR_IPC_NOT_IMPLEMENTED	= 608,	/**< Operation not implemented (for example when using
						     some file systems).				*/
	P_ERROR_IPC_DEADLOCK		= 609,	/**< Deadlock detected.					*/
	P_ERROR_IPC_FAILED		= 610	/**< General error.					*/
} PErrorIPC;

/** Opaque data structure for an error object. */
typedef struct _PError PError;

/**
 * @brief Initializes new empty #PError.
 * @return Newly initialized #PError object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PError *	p_error_new		();

/**
 * @brief Initializes new #PError with data.
 * @param code Error code.
 * @param native_code Native error code, leave 0 to ignore.
 * @param message Error message.
 * @return Newly initialized #PError object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PError *	p_error_new_literal	(pint		code,
						 pint		native_code,
						 const pchar	*message);

/**
 * @brief Gets error message.
 * @param error #PError object to get message from.
 * @return Error message in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API const pchar *	p_error_get_message	(PError		*error);

/**
 * @brief Gets error code.
 * @param error #PError object to get code from.
 * @return Error code in case of success, 0 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint		p_error_get_code	(PError		*error);

/**
 * @brief Gets platform native error code, if any.
 * @param error #PError object to get native code from.
 * @return Error code in case of success, 0 otherwise.
 * @since 0.0.1
 * @note In some situations there can be no native code error,
 * i.e. when internal library call failed. Do not rely on this code.
 */
P_LIB_API pint		p_error_get_native_code	(PError		*error);

/**
 * @brief Gets error domain.
 * @param error #PError object to get domain from.
 * @return Error domain in case of success, #P_ERROR_DOMAIN_NONE otherwise.
 * @since 0.0.1
 */
P_LIB_API PErrorDomain	p_error_get_domain	(PError		*error);

/**
 * @brief Creates a copy of a #PError object.
 * @param error #PError object to copy.
 * @return Newly created #PError object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 * @note The caller is responsible to free memory of the created object
 * after the usage.
 */
P_LIB_API PError *	p_error_copy		(PError		*error);

/**
 * @brief Sets error data.
 * @param error #PError object to set data for.
 * @param code Error code.
 * @param native_code Native error code, leave 0 to ignore.
 * @param message Error message.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_error	(PError		*error,
						 pint		code,
						 pint		native_code,
						 const pchar	*message);

/**
 * @brief Sets error data through the double pointer.
 * @param error #PError object to set data for.
 * @param code Error code.
 * @param native_code Native error code, leave 0 to ignore.
 * @param message Error message.
 * @since 0.0.1
 *
 * If @a error is NULL it does nothing. If @a error is not NULL then
 * @a *error should be NULL, otherwise it does nothing. It creates
 * #PError object, sets error data and assigns it to @a *error. The caller
 * is responsible to free memory of the created object after the usage.
 */
P_LIB_API void		p_error_set_error_p	(PError		**error,
						 pint		code,
						 pint		native_code,
						 const pchar	*message);

/**
 * @brief Sets error code.
 * @param error #PError object to set data for.
 * @param code Error code.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_code	(PError		*error,
						 pint		code);

/**
 * @brief Sets platform native error code.
 * @param error #PError object to set data for.
 * @param native_code Platform native error code.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_native_code	(PError		*error,
						 pint		native_code);

/**
 * @brief Sets error message.
 * @param error #PError object to set data for.
 * @param message Error message.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_message	(PError		*error,
						 const pchar	*message);

/**
 * @brief Clears error data.
 * @param error #PError object to clear data for.
 * @since 0.0.1
 * @note Error code is reseted to 0.
 */
P_LIB_API void		p_error_clear		(PError		*error);

/**
 * @brief Frees previously initialized error object.
 * @param error #PError object to free.
 * @since 0.0.1
 */
P_LIB_API void		p_error_free		(PError		*error);

P_END_DECLS

#endif /* __PERROR_H__ */
