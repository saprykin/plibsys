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

/**
 * @file perror.h
 * @brief Error report system
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
 * pointer into an API call. Simply initialize it with zero and check the result
 * after. Therefore you need to free memory if an error occurred.
 *
 * Most operating systems store the last error code of the most system calls in
 * a thread-specific variable. Moreover, Windows stores the error code of the
 * last socket related call in a separate variable. Use
 * p_error_get_last_system() and p_error_set_last_system() to access the last
 * system error code, p_error_get_last_net() and p_error_set_last_net() to
 * access the last network error code.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PERROR_H
#define PLIBSYS_HEADER_PERROR_H

#include <pmacros.h>
#include <ptypes.h>
#include <perrortypes.h>

P_BEGIN_DECLS

/** Opaque data structure for an error object. */
typedef struct PError_ PError;

/**
 * @brief Initializes a new empty #PError.
 * @return Newly initialized #PError object in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PError *	p_error_new		(void);

/**
 * @brief Initializes a new #PError with data.
 * @param code Error code.
 * @param native_code Native error code, leave 0 to ignore.
 * @param message Error message.
 * @return Newly initialized #PError object in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PError *	p_error_new_literal	(pint		code,
						 pint		native_code,
						 const pchar	*message);

/**
 * @brief Gets an error message.
 * @param error #PError object to get the message from.
 * @return Error message in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API const pchar *	p_error_get_message	(PError		*error);

/**
 * @brief Gets an error code.
 * @param error #PError object to get the code from.
 * @return Error code in case of success, 0 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint		p_error_get_code	(PError		*error);

/**
 * @brief Gets a platform native error code, if any.
 * @param error #PError object to get the native code from.
 * @return Error code in case of success, 0 otherwise.
 * @since 0.0.1
 * @note In some situations there can be no native code error, i.e. when an
 * internal library call failed. Do not rely on this code.
 */
P_LIB_API pint		p_error_get_native_code	(PError		*error);

/**
 * @brief Gets an error domain.
 * @param error #PError object to get the domain from.
 * @return Error domain in case of success, #P_ERROR_DOMAIN_NONE otherwise.
 * @since 0.0.1
 */
P_LIB_API PErrorDomain	p_error_get_domain	(PError		*error);

/**
 * @brief Creates a copy of a #PError object.
 * @param error #PError object to copy.
 * @return Newly created #PError object in case of success, NULL otherwise.
 * @since 0.0.1
 * @note The caller is responsible to free memory of the created object after
 * usage.
 */
P_LIB_API PError *	p_error_copy		(PError		*error);

/**
 * @brief Sets error data.
 * @param error #PError object to set the data for.
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
 * @brief Sets error data through a double pointer.
 * @param error #PError object to set the data for.
 * @param code Error code.
 * @param native_code Native error code, leave 0 to ignore.
 * @param message Error message.
 * @since 0.0.1
 *
 * If @a error is NULL it does nothing. If @a error is not NULL then @a *error
 * should be NULL, otherwise it does nothing. It creates a #PError object, sets
 * error data and assigns it to @a *error. The caller is responsible to free
 * memory of the created object after usage.
 */
P_LIB_API void		p_error_set_error_p	(PError		**error,
						 pint		code,
						 pint		native_code,
						 const pchar	*message);

/**
 * @brief Sets an error code.
 * @param error #PError object to set the code for.
 * @param code Error code.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_code	(PError		*error,
						 pint		code);

/**
 * @brief Sets a platform native error code.
 * @param error #PError object to set the native error code for.
 * @param native_code Platform native error code.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_native_code	(PError		*error,
						 pint		native_code);

/**
 * @brief Sets an error message.
 * @param error #PError object to set the message for.
 * @param message Error message.
 * @since 0.0.1
 */
P_LIB_API void		p_error_set_message	(PError		*error,
						 const pchar	*message);

/**
 * @brief Clears error data.
 * @param error #PError object to clear the data for.
 * @since 0.0.1
 * @note Error code is reseted to 0.
 */
P_LIB_API void		p_error_clear		(PError		*error);

/**
 * @brief Frees a previously initialized error object.
 * @param error #PError object to free.
 * @since 0.0.1
 */
P_LIB_API void		p_error_free		(PError		*error);

/**
 * @brief Gets the last system native error code.
 * @return Last system native error code.
 * @since 0.0.2
 * @sa p_error_get_last_net(), p_error_set_last_system(),
 * p_error_set_last_net()
 * @note If you want get an error code for socket-related calls, use
 * p_error_get_last_net() instead.
 */

P_LIB_API pint		p_error_get_last_system	(void);

/**
 * @brief Gets the last network native error code.
 * @return Last network native error code.
 * @since 0.0.2
 * @sa p_error_get_last_system(), p_error_set_last_net(),
 * p_error_set_last_system()
 */
P_LIB_API pint		p_error_get_last_net	(void);

/**
 * @brief Sets the last system native error code.
 * @param code Error code to set.
 * @since 0.0.2
 * @sa p_error_set_last_net(), p_error_get_last_system(),
 * p_error_get_last_net()
 * @note If you want set an error code for socket-related calls, use
 * p_error_set_last_net() instead.
 */
P_LIB_API void		p_error_set_last_system	(pint code);

/**
 * @brief Sets the last network native error code.
 * @param code Error code to set.
 * @since 0.0.2
 * @sa p_error_set_last_system(), p_error_get_last_net(),
 * p_error_get_last_system()
 */
P_LIB_API void		p_error_set_last_net	(pint code);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PERROR_H */
