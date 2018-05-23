/*
 * The MIT License
 *
 * Copyright (C) 2011-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
