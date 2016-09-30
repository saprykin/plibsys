/*
 * Copyright (C) 2011-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file pstring.h
 * @brief String manipulation routines
 * @author Alexander Saprykin
 *
 * Strings are represented as a sequence of single-byte characters (from the
 * ASCII table) with the trailing zero character (\0).
 *
 * Some useful string manipulation routines are represented here.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSTRING_H
#define PLIBSYS_HEADER_PSTRING_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/**
 * @brief Copies a string.
 * @param str String with the trailing zero to copy.
 * @return Copy of the @a str in case of success, NULL otherwise. The caller
 * takes ownership of the returned string.
 * @since 0.0.1
 */
P_LIB_API pchar *	p_strdup	(const pchar	*str);

/**
 * @brief Removes trailing and leading whitespaces.
 * @param str String with the trailing zero to process.
 * @return Newlly allocated string in case of success, NULL otherwise. The
 * caller takes ownership of the returned string.
 * @since 0.0.1
 */
P_LIB_API pchar *	p_strchomp	(const pchar	*str);

/**
 * @brief Tokenizes a string by given delimiters.
 * @param[in,out] str String to tokenize.
 * @param delim List of delimiters to split the string.
 * @param buf Context to store tokenize info.
 * @return Pointer to a splitted zero-terminated string in case of success, NULL
 * otherwise.
 * @since 0.0.1
 * @note The @a str is modified by this call, so take care for that. The
 * returned pointer points on a @a str substring, so you do not need to call
 * p_free() on it.
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
 * Take attention that you need to pass the original string only once, after
 * that you should pass NULL instead. You can also pass different delimiters
 * each time.
 *
 * Some platforms do not support the third parameter and it can be remained
 * unused. In that case this call wouldn't be thread-safe.
 */
P_LIB_API pchar *	p_strtok	(pchar		*str,
					 const pchar	*delim,
					 pchar		**buf);

/**
 * @brief Converts a string to @a double without a locale dependency.
 * @param str String to convert.
 * @return Floating point value in case of success, 0 otherwise.
 * @since 0.0.1
 *
 * Since the atof() system call is locale dependent, you can use this call to
 * convert string variables to @a double values. The decimal point is '.' as in
 * the 'C' locale.
 */
P_LIB_API double	p_strtod	(const pchar	*str);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PSTRING_H */
