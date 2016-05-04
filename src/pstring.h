/*
 * Copyright (C) 2011-2016 Alexander Saprykin <xelfium@gmail.com>
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file pstring.h
 * @brief Strings manipulation routines
 * @author Alexander Saprykin
 *
 * Here are routines to manipulate with strings consist of chars and trailing \0.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PSTRING_H__
#define __PSTRING_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/**
 * @brief Copies string.
 * @param str String with trailing \0 to copy.
 * @return Copy of the @a str in case of success, NULL otherwise. Caller takes
 * ownership for the returned string.
 * @since 0.0.1
 */
P_LIB_API pchar *	p_strdup	(const pchar *str);

/**
 * @brief Removes trailing and leading whitespaces.
 * @param str String with trailing \0 to process.
 * @return Newlly allocated string in case of success, NULL otherwise. Caller takes
 * ownership for the returned string.
 * @since 0.0.1
 */
P_LIB_API pchar *	p_strchomp	(const pchar *str);

/**
 * @brief Tokenizes string by given delimiters.
 * @param[in, out] str String to tokenize.
 * @param delim List of delimiters to split the string.
 * @param buf Context to store tokenize info.
 * @return Pointer to splitted zero-terminated string in case of success, NULL
 * otherwise.
 * @since 0.0.1
 * @note @a str is modified by this call, so take care for that.
 * Returned pointer points on the @a str substring, so you do not need
 * to call p_free() on it.
 *
 * The common usage of this call is following:
 * @code
 * pchar *token, *buf;
 * pchar str[] = "This is a test string"
 * pchar delim[] = " \t"
 * ...
 * token = p_strtok (str, delim, &buf);
 *
 * while (token != NULL) {
 *     printf ("Splitted string: %s\n", token);
 *     token = p_strtok (NULL, delim, &buf);
 * }
 * @endcode
 * Take an attention that you need to pass original string only
 * once, then you should pass NULL instead. You can also pass
 * different delimiters each time.
 *
 * Some platforms do not support third parameter and it can be
 * remained unused. In that case this call wouldn't be thread-safe.
 */
P_LIB_API pchar *	p_strtok	(pchar		*str,
					 const pchar	*delim,
					 pchar		**buf);

/**
 * @brief Converts string to @a double without locale dependency
 * @param str String to convert.
 * @return Floating point value in case of success, 0 otherwise.
 * @since 0.0.1
 *
 * Since atof() system call is locale dependent, you can use this
 * call to convert string variables to @a double values. The
 * decimal point is '.' as in 'C' locale.
 */
P_LIB_API double	p_strtod	(const pchar	*str);

P_END_DECLS

#endif /* __PSTRING_H__ */
