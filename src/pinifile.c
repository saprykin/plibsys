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

#include "perror.h"
#include "pinifile.h"
#include "plist.h"
#include "pmem.h"
#include "pstring.h"
#include "perror-private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define	P_INI_FILE_MAX_LINE	1024

typedef struct PIniParameter_ {
	pchar		*name;
	pchar		*value;
} PIniParameter;

typedef struct PIniSection_ {
	pchar		*name;
	PList		*keys;
} PIniSection;

struct PIniFile_ {
	pchar		*path;
	PList		*sections;
	pboolean	is_parsed;
};

static PIniParameter * pp_ini_file_parameter_new (const pchar *name, const pchar *val);
static void pp_ini_file_parameter_free (PIniParameter *param);
static PIniSection * pp_ini_file_section_new (const pchar *name);
static void pp_ini_file_section_free (PIniSection *section);
static pchar * pp_ini_file_find_parameter (const PIniFile *file, const pchar *section, const pchar *key);

static PIniParameter *
pp_ini_file_parameter_new (const pchar	*name,
			   const pchar	*val)
{
	PIniParameter *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PIniParameter))) == NULL))
		return NULL;

	if (P_UNLIKELY ((ret->name = p_strdup (name)) == NULL)) {
		p_free (ret);
		return NULL;
	}

	if (P_UNLIKELY ((ret->value = p_strdup (val)) == NULL)) {
		p_free (ret->name);
		p_free (ret);
		return NULL;
	}

	return ret;
}

static void
pp_ini_file_parameter_free (PIniParameter *param)
{
	p_free (param->name);
	p_free (param->value);
	p_free (param);
}

static PIniSection *
pp_ini_file_section_new (const pchar *name)
{
	PIniSection *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PIniSection))) == NULL))
		return NULL;

	if (P_UNLIKELY ((ret->name = p_strdup (name)) == NULL)) {
		p_free (ret);
		return NULL;
	}

	return ret;
}

static void
pp_ini_file_section_free (PIniSection *section)
{
	p_list_foreach (section->keys, (PFunc) pp_ini_file_parameter_free, NULL);
	p_list_free (section->keys);
	p_free (section->name);
	p_free (section);
}

static pchar *
pp_ini_file_find_parameter (const PIniFile *file, const pchar *section, const pchar *key)
{
	PList	*item;

	if (P_UNLIKELY (file == NULL || file->is_parsed == FALSE || section == NULL || key == NULL))
		return NULL;

	for (item = file->sections; item != NULL; item = item->next)
		if (strcmp (((PIniSection *) item->data)->name, section) == 0)
			break;

	if (item == NULL)
		return NULL;

	for (item = ((PIniSection *) item->data)->keys; item != NULL; item = item->next)
		if (strcmp (((PIniParameter *) item->data)->name, key) == 0)
			return p_strdup (((PIniParameter *) item->data)->value);

	return NULL;
}

P_LIB_API PIniFile *
p_ini_file_new (const pchar *path)
{
	PIniFile	*ret;

	if (P_UNLIKELY (path == NULL))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PIniFile))) == NULL))
		return NULL;

	if (P_UNLIKELY ((ret->path = p_strdup (path)) == NULL)) {
		p_free (ret);
		return NULL;
	}

	ret->is_parsed = FALSE;

	return ret;
}

P_LIB_API void
p_ini_file_free (PIniFile *file)
{
	if (P_UNLIKELY (file == NULL))
		return;

	p_list_foreach (file->sections, (PFunc) pp_ini_file_section_free, NULL);
	p_list_free (file->sections);
	p_free (file->path);
	p_free (file);
}

P_LIB_API pboolean
p_ini_file_parse (PIniFile	*file,
		  PError	**error)
{
	PIniSection	*section;
	PIniParameter	*param;
	FILE		*in_file;
	pchar		*dst_line, *tmp_str;
	pchar		src_line[P_INI_FILE_MAX_LINE + 1],
			key[P_INI_FILE_MAX_LINE + 1],
			value[P_INI_FILE_MAX_LINE + 1];
	pint		bom_shift;

	if (P_UNLIKELY (file == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (file->is_parsed)
		return TRUE;

	if (P_UNLIKELY ((in_file = fopen (file->path, "r")) == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to open file for reading");
		return FALSE;
	}

	dst_line = NULL;
	section  = NULL;
	param    = NULL;

	memset (src_line, 0, sizeof (src_line));

	while (fgets (src_line, sizeof (src_line), in_file) != NULL) {
		/* UTF-8, UTF-16 and UTF-32 BOM detection */
		if ((puchar) src_line[0] == 0xEF && (puchar) src_line[1] == 0xBB && (puchar) src_line[2] == 0xBF)
			bom_shift = 3;
		else if (((puchar) src_line[0] == 0xFE && (puchar) src_line[1] == 0xFF) ||
			 ((puchar) src_line[0] == 0xFF && (puchar) src_line[1] == 0xFE))
			bom_shift = 2;
		else if ((puchar) src_line[0] == 0x00 && (puchar) src_line[1] == 0x00 &&
			 (puchar) src_line[2] == 0xFE && (puchar) src_line[3] == 0xFF)
			bom_shift = 4;
		else if ((puchar) src_line[0] == 0xFF && (puchar) src_line[1] == 0xFE &&
			 (puchar) src_line[2] == 0x00 && (puchar) src_line[3] == 0x00)
			bom_shift = 4;
		else
			bom_shift = 0;

		dst_line = p_strchomp (src_line + bom_shift);

		if (dst_line == NULL)
			continue;

		/* This should not happen */
		if (P_UNLIKELY (strlen (dst_line) > P_INI_FILE_MAX_LINE))
			dst_line[P_INI_FILE_MAX_LINE] = '\0';

		if (dst_line[0] == '[' && dst_line[strlen (dst_line) - 1] == ']' &&
		    sscanf (dst_line, "[%[^]]", key) == 1) {
			/* New section found */
			if ((tmp_str = p_strchomp (key)) != NULL) {
				/* This should not happen */
				if (P_UNLIKELY (strlen (tmp_str) > P_INI_FILE_MAX_LINE))
					tmp_str[P_INI_FILE_MAX_LINE] = '\0';

				strcpy (key, tmp_str);
				p_free (tmp_str);

				if (section != NULL) {
					if (section->keys == NULL)
						pp_ini_file_section_free (section);
					else
						file->sections = p_list_prepend (file->sections, section);
				}

				section = pp_ini_file_section_new (key);
			}
		} else if (sscanf (dst_line, "%[^=] = \"%[^\"]\"", key, value) == 2 ||
			   sscanf (dst_line, "%[^=] = '%[^\']'", key, value) == 2 ||
			   sscanf (dst_line, "%[^=] = %[^;#]", key, value) == 2) {
			/* New parameter found */
			if ((tmp_str = p_strchomp (key)) != NULL) {
				/* This should not happen */
				if (P_UNLIKELY (strlen (tmp_str) > P_INI_FILE_MAX_LINE))
					tmp_str[P_INI_FILE_MAX_LINE] = '\0';

				strcpy (key, tmp_str);
				p_free (tmp_str);

				if ((tmp_str = p_strchomp (value)) != NULL) {
					/* This should not happen */
					if (P_UNLIKELY (strlen (tmp_str) > P_INI_FILE_MAX_LINE))
						tmp_str[P_INI_FILE_MAX_LINE] = '\0';

					strcpy (value, tmp_str);
					p_free (tmp_str);

					if (strcmp (value, "\"\"") == 0 || (strcmp (value, "''") == 0))
						value[0] = '\0';

					if (section != NULL && (param = pp_ini_file_parameter_new (key, value)) != NULL)
						section->keys = p_list_prepend (section->keys, param);
				}
			}
		}

		p_free (dst_line);
		memset (src_line, 0, sizeof (src_line));
	}

	if (section != NULL) {
		if (section->keys == NULL)
			pp_ini_file_section_free (section);
		else
			file->sections = p_list_append (file->sections, section);
	}

	if (P_UNLIKELY (fclose (in_file) != 0))
		P_WARNING ("PIniFile::p_ini_file_parse: fclose() failed");

	file->is_parsed = TRUE;

	return TRUE;
}

P_LIB_API pboolean
p_ini_file_is_parsed (const PIniFile *file)
{
	if (P_UNLIKELY (file == NULL))
		return FALSE;

	return file->is_parsed;
}

P_LIB_API PList *
p_ini_file_sections (const PIniFile *file)
{
	PList	*ret;
	PList	*sec;

	if (P_UNLIKELY (file == NULL || file->is_parsed == FALSE))
		return NULL;

	ret = NULL;

	for (sec = file->sections; sec != NULL; sec = sec->next)
		ret = p_list_prepend (ret, p_strdup (((PIniSection *) sec->data)->name));

	return ret;
}

P_LIB_API PList *
p_ini_file_keys (const PIniFile	*file,
		 const pchar	*section)
{
	PList	*ret;
	PList	*item;

	if (P_UNLIKELY (file == NULL || file->is_parsed == FALSE || section == NULL))
		return NULL;

	ret = NULL;

	for (item = file->sections; item != NULL; item = item->next)
		if (strcmp (((PIniSection *) item->data)->name, section) == 0)
			break;

	if (item == NULL)
		return NULL;

	for (item = ((PIniSection *) item->data)->keys; item != NULL; item = item->next)
		ret = p_list_prepend (ret, p_strdup (((PIniParameter *) item->data)->name));

	return ret;
}

P_LIB_API pboolean
p_ini_file_is_key_exists (const PIniFile	*file,
			  const pchar		*section,
			  const pchar		*key)
{
	PList	*item;

	if (P_UNLIKELY (file == NULL || file->is_parsed == FALSE || section == NULL || key == NULL))
		return FALSE;

	for (item = file->sections; item != NULL; item = item->next)
		if (strcmp (((PIniSection *) item->data)->name, section) == 0)
			break;

	if (item == NULL)
		return FALSE;

	for (item = ((PIniSection *) item->data)->keys; item != NULL; item = item->next)
		if (strcmp (((PIniParameter *) item->data)->name, key) == 0)
			return TRUE;

	return FALSE;
}

P_LIB_API pchar *
p_ini_file_parameter_string (const PIniFile	*file,
			     const pchar	*section,
			     const pchar	*key,
			     const pchar	*default_val)
{
	pchar *val;

	if ((val = pp_ini_file_find_parameter (file, section, key)) == NULL)
		return p_strdup (default_val);

	return val;
}

P_LIB_API pint
p_ini_file_parameter_int (const PIniFile	*file,
			  const pchar		*section,
			  const pchar		*key,
			  pint			default_val)
{
	pchar	*val;
	pint	ret;

	if ((val = pp_ini_file_find_parameter (file, section, key)) == NULL)
		return default_val;

	ret = atoi (val);
	p_free (val);

	return ret;
}

P_LIB_API double
p_ini_file_parameter_double (const PIniFile	*file,
			     const pchar	*section,
			     const pchar	*key,
			     double		default_val)
{
	pchar	*val;
	pdouble	ret;

	if ((val = pp_ini_file_find_parameter (file, section, key)) == NULL)
		return default_val;

	ret = p_strtod (val);
	p_free (val);

	return ret;
}

P_LIB_API pboolean
p_ini_file_parameter_boolean (const PIniFile	*file,
			      const pchar	*section,
			      const pchar	*key,
			      pboolean		default_val)
{
	pchar		*val;
	pboolean	ret;

	if ((val = pp_ini_file_find_parameter (file, section, key)) == NULL)
		return default_val;

	if (strcmp (val, "true") == 0 || strcmp (val, "TRUE") == 0)
		ret = TRUE;
	else if (strcmp (val, "false") == 0 || strcmp (val, "FALSE") == 0)
		ret = FALSE;
	else if (atoi (val) > 0)
		ret = TRUE;
	else
		ret = FALSE;

	p_free (val);

	return ret;
}

P_LIB_API PList *
p_ini_file_parameter_list (const PIniFile	*file,
			   const pchar		*section,
			   const pchar		*key)
{
	PList	*ret = NULL;
	pchar	*val, *str;
	pchar	buf[P_INI_FILE_MAX_LINE + 1];
	psize	len, buf_cnt;

	if ((val = pp_ini_file_find_parameter (file, section, key)) == NULL)
		return NULL;

	len = strlen (val);

	if (len < 3 || val[0] != '{' || val[len - 1] != '}') {
		p_free (val);
		return NULL;
	}

	/* Skip first brace '{' symbol */
	str = val + 1;
	buf[0] = '\0';
	buf_cnt = 0;

	while (*str && *str != '}') {
		if (!isspace (* ((const puchar *) str)))
			buf[buf_cnt++] = *str;
		else {
			buf[buf_cnt] = '\0';

			if (buf_cnt > 0)
				ret = p_list_append (ret, p_strdup (buf));

			buf_cnt = 0;
		}

		++str;
	}

	if (buf_cnt > 0) {
		buf[buf_cnt] = '\0';
		ret = p_list_append (ret, p_strdup (buf));
	}

	p_free (val);

	return ret;
}
