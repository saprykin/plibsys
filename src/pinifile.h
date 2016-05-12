/*
 * Copyright (C) 2012-2016 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pinifile.h
 * @brief INI file parser
 * @author Alexander Saprykin
 *
 * INI file is usually used for storing configuration information. It consists
 * of sections: every section starts with a line containing the name in square
 * brackets (i.e. [section_name]). After that line all the following parameters
 * will belong to that section until another section begins.
 *
 * Each section has a list of key-value pairs. Empty sections are not permitted
 * (they will be skipped). Every key-value pair is represented with a line in
 * a `key = value` format. If section has several values with the same key the
 * last one will be used. Value is parsed by the first in-order '=' symbol. All
 * the following '=' occurrences belong to the value.
 *
 * All symbols after '#' and ';' (even at the line ending) are the comments and
 * wouldn't be read. If you want to use them in values take the value inside ""
 * or '' symbols. Section name line are not allowed to use comment symbols after
 * the section name in square brackets itself.
 *
 * Integer values can be written in a usual form.
 *
 * Floating point values can be written in any commonly used notation (i.e.
 * with decimal point, in exponential form using 'e' character). The only valid
 * decimal point symbol is a '.'. There is no locale dependency on a decimal
 * point.
 *
 * Boolean values can be written in a form of 'true/false' or 'TRUE/FALSE', or
 * simply '0/1'.
 *
 * Any value can be interpreted as a string at any moment. Actually all the
 * values are stored internally as strings.
 *
 * List of values can be stored between {} symbols separated with spaces. List
 * only supports string values, so you should convert them to numbers manually.
 * List doesn't support strings with spaces - such strings will be splitted.
 *
 * To parse a file, create #PIniFile with p_ini_file_new() and then parse it
 * with p_ini_file_parse() routine.
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

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PINIFILE_H__
#define __PINIFILE_H__

#include <pmacros.h>
#include <ptypes.h>
#include <plist.h>
#include <perror.h>

P_BEGIN_DECLS

/** INI file opaque data structure. */
typedef struct _PIniFile PIniFile;

/**
 * @brief Creates a new #PIniFile for parsing.
 * @param path Path to file to parse.
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
 * @return TRUE if file was already parsed, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_is_parsed		(const PIniFile	*file);

/**
 * @brief Gets all sections from the given #PIniFile file.
 * @param file #PIniFile to get sections from. @a file should be parsed before.
 * @return #PList of section names.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free returned list with p_list_free().
 */
P_LIB_API PList	*	p_ini_file_sections		(const PIniFile	*file);

/**
 * @brief Gets all keys from the given #PIniFile file's section.
 * @param file #PIniFile to get keys from. @a file should be parsed before.
 * @param section Section name to get keys from.
 * @return #PList of key names.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free returned list with p_list_free().
 */
P_LIB_API PList *	p_ini_file_keys			(const PIniFile	*file,
							 const pchar	*section);

/**
 * @brief Checks whether the key exists.
 * @param file #PIniFile to check in. @a file should be parsed before.
 * @param section Section to check key in.
 * @param key Key to check.
 * @return TRUE if @a key exists, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_is_key_exists	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key);

/**
 * @brief Gets specified parameter's value as a string. @a file should be parsed
 * before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() returned string after usage.
 */
P_LIB_API pchar *	p_ini_file_parameter_string	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 const pchar	*default_val);

/**
 * @brief Gets specified parameter's value as an integer. @a file should
 * be parsed before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */
P_LIB_API pint		p_ini_file_parameter_int	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 pint		default_val);

/**
 * @brief Gets specified parameter's value as a floating point. @a file should
 * be parsed before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */
P_LIB_API double	p_ini_file_parameter_double	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 double		default_val);
/**
 * @brief Gets specified parameter's value as a boolean. @a file should
 * be parsed before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise.
 * @since 0.0.1
 */

P_LIB_API pboolean	p_ini_file_parameter_boolean	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key,
							 pboolean	default_val);

/**
 * @brief Gets specified parameter's value as a list of strings separated
 * with the spaces or tabs. @a file should be parsed before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @return #PList of strings. NULL will be returned if no parameter with the
 * given name exists.
 * @since 0.0.1
 * @note It's a caller responsibility to p_free() each returned string and to
 * free returned list with p_list_free().
 */
P_LIB_API PList *	p_ini_file_parameter_list	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key);

P_END_DECLS

#endif /* __PINIFILE_H__ */
