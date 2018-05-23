/*
 * The MIT License
 *
 * Copyright (C) 2012-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file pinifile.h
 * @brief INI file parser
 * @author Alexander Saprykin
 *
 * An INI file is usually used for storing configuration information. It
 * consists of sections: every section starts with a line containing the name in
 * square brackets (i.e. [section_name]). After that line all the following
 * parameters will belong to that section until another section begins.
 *
 * Each section has a list of key-value pairs. Empty sections are not permitted
 * (they will be skipped). Every key-value pair is represented with a line in
 * the `key = value` format. If a section has several values with the same key
 * the last one will be used. A value is parsed by the first in-order '='
 * symbol. All the following '=' occurrences belong to the value.
 *
 * All symbols after '#' and ';' (even at the line ending) are the comments and
 * wouldn't be read. If you want to use them in values take the value inside the
 * "" or '' symbols. A section name line is not allowed to use the comment
 * symbols after the section name in the square brackets itself.
 *
 * Integer values can be written in the usual form.
 *
 * Floating point values can be written in any commonly used notation (i.e. with
 * the decimal point, in the exponential form using the 'e' character). The only
 * valid decimal point symbol is the '.'. There is no locale dependency on the
 * decimal point.
 *
 * Boolean values can be written in the form of 'true/false' or 'TRUE/FALSE', or
 * simply '0/1'.
 *
 * Any value can be interpreted as a string at any moment. Actually all the
 * values are stored internally as strings.
 *
 * A list of values can be stored between the '{}' symbols separated with
 * spaces. The list only supports string values, so you should convert them to
 * numbers manually. The list doesn't support strings with spaces - such strings
 * will be splitted.
 *
 * To parse a file, create #PIniFile with p_ini_file_new() and then parse it
 * with the p_ini_file_parse() routine.
 *
 * #PIniFile handles (skips) UTF-8/16/32 BOM characters (marks).
 *
 * Example of the INI file contents:
 * @code
 * [numeric_section]
 * numeric_value_1 = 1234 # One type of the comment
 * numeric_value_2 = 123  ; Comment is allowed here
 *
 * [floating_section]
 * float_value_1 = 123.3e10
 * float_value_2 = 123.19
 *
 * [boolean_section]
 * boolean_value_1 = TRUE
 * boolean_value_2 = 0
 * boolean_value_3 = false
 *
 * [string_section]
 * string_value_1 = "Test string"
 * string_value_2 = 'Another test string'
 *
 * [list_section]
 * list_value = {123 val 7654}
 * @endcode
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PINIFILE_H
#define PLIBSYS_HEADER_PINIFILE_H

#include <pmacros.h>
#include <ptypes.h>
#include <plist.h>
#include <perror.h>

P_BEGIN_DECLS

/** INI file opaque data structure. */
typedef struct PIniFile_ PIniFile;

/**
 * @brief Creates a new #PIniFile for parsing.
 * @param path Path to a file to parse.
 * @return Newly allocated #PIniFile in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PIniFile *	p_ini_file_new			(const pchar	*path);

/**
 * @brief Frees memory and allocated resources of #PIniFile.
 * @param file #PIniFile to free.
 * @since 0.0.1
 */
P_LIB_API void		p_ini_file_free			(PIniFile	*file);

/**
 * @brief Parses given #PIniFile.
 * @param file #PIniFile file to parse.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_parse		(PIniFile	*file,
							 PError		**error);

/**
 * @brief Checks whether #PIniFile was already parsed or not.
 * @param file #PIniFile to check.
 * @return TRUE if the file was already parsed, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_is_parsed		(const PIniFile	*file);

/**
 * @brief Gets all the sections from a given file.
 * @param file #PIniFile to get the sections from. The @a file should be parsed
 * before.
 * @return #PList of section names.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free the returned list with p_list_free().
 */
P_LIB_API PList	*	p_ini_file_sections		(const PIniFile	*file);

/**
 * @brief Gets all the keys from a given section.
 * @param file #PIniFile to get the keys from. The @a file should be parsed
 * before.
 * @param section Section name to get the keys from.
 * @return #PList of key names.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free the returned list with p_list_free().
 */
P_LIB_API PList *	p_ini_file_keys			(const PIniFile	*file,
							 const pchar	*section);

/**
 * @brief Checks whether a key exists.
 * @param file #PIniFile to check in. The @a file should be parsed before.
 * @param section Section to check the key in.
 * @param key Key to check.
 * @return TRUE if @a key exists, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_is_key_exists	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key);

/**
 * @brief Gets specified parameter's value as a string.
 * @param file #PIniFile to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() the returned string after
 * usage.
 */
P_LIB_API pchar *	p_ini_file_parameter_string	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 const pchar	*default_val);

/**
 * @brief Gets specified parameter's value as an integer.
 * @param file #PIniFile to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */
P_LIB_API pint		p_ini_file_parameter_int	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 pint		default_val);

/**
 * @brief Gets specified parameter's value as a floating point.
 * @param file #PIniFile to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */
P_LIB_API double	p_ini_file_parameter_double	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 double		default_val);
/**
 * @brief Gets specified parameter's value as a boolean.
 * @param file #PIniFile to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */

P_LIB_API pboolean	p_ini_file_parameter_boolean	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 pboolean	default_val);

/**
 * @brief Gets specified parameter's value as a list of strings separated with
 * the spaces or tabs.
 * @param file #PIniFile to get the value from. The @a file should be parsed
 * before.
 * @param section Section to get the value from.
 * @param key Key to get the value from.
 * @return #PList of strings. NULL will be returned if no parameter with the
 * given name exists.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free the returned list with p_list_free().
 */
P_LIB_API PList *	p_ini_file_parameter_list	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PINIFILE_H */
