/* 
 * Copyright (C) 2012-2013 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pinifile.h
 * @brief INI files parser
 * @author Alexander Saprykin
 *
 * #PIniFile is an INI files parser which can be useful for reading
 * configuration files. INI file is splitted to sections using [] braces
 * and section name within them. After each section name there are
 * parameters in form 'key = value'. All symbols after '#' and ';'
 * are comments and wouldn't be read. If you want to use them in values
 * take value inside "" or '' symbols. Parameters related to section
 * until another section name is occured.
 * To parse file, create #PIniFile with p_ini_file_new() and then
 * parse it with p_ini_file_parse() functions. Boolean values can
 * be written in form of 'true/false' or 'TRUE/FALSE', or simply '0/1'.
 * @warning Be carefull that UTF-8 BOM couldn't be recognized while parsing.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PINIFILE_H__
#define __PINIFILE_H__

#include <pmacros.h>
#include <ptypes.h>
#include <plist.h>

P_BEGIN_DECLS

/** PIniFile opaque data struct */
typedef struct _PIniFile PIniFile;

/**
 * @brief Creates new #PIniFile for parsing.
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
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_parse		(PIniFile	*file);

/**
 * @brief Check whether #PIniFile was already parsed or not.
 * @param file #PIniFile to check.
 * @return TRUE if file was already parsed, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_ini_file_is_parsed		(const PIniFile	*file);

/**
 * @brief Gets all sections from given #PIniFile file.
 * @param file #PIniFile to get sections from. @a file should be
 * parsed before.
 * @return #PList of section names. It's a caller responsibility
 * to p_free() each string and to free returned list with p_list_free().
 * @since 0.0.1
 */
P_LIB_API PList	*	p_ini_file_sections		(const PIniFile	*file);

/**
 * @brief Gets all keys from given #PIniFile file's section.
 * @param file #PIniFile to get keys from. @a file should be
 * parsed before.
 * @param section Section name to get keys from.
 * @return #PList of key names. It's a caller responsibility
 * to p_free() each string and to free returned list with p_list_free().
 * @since 0.0.1
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
 * @brief Gets specified parameter's value as a string. @a file should
 * be parsed before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @param default_val Default value to return if no specified key exists.
 * @return Key's value in case of success, @a default_value otherwise. It's
 * a caller responsibility to p_free() string after usage.
 * @since 0.0.1
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
 * with space or tabs. @a file should be parsed before.
 * @param file #PIniFile to get value from.
 * @param section Section to get from.
 * @param key Key to get value from.
 * @return #PList of strings. It's a caller responsibility
 * to p_free() each string and to free returned list with p_list_free().
 * NULL will be returned if no parameter with the given name exists.
 * @since 0.0.1
 */
P_LIB_API PList *	p_ini_file_parameter_list	(const PIniFile	*file,
							 const pchar	*section,
							 const pchar	*key);

P_END_DECLS

#endif /* __PINIFILE_H__ */
