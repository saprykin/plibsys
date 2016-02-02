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

#include "pmem.h"
#include "perror.h"
#include "pstring.h"

struct _PError {
	pint	code;
	pchar	*message;
};

P_LIB_API PError *
p_error_new ()
{
	PError *ret;

	if ((ret = p_malloc0 (sizeof (PError))) == NULL)
		return NULL;

	return ret;
}

P_LIB_API PError *
p_error_new_literal (pint	code,
		    const pchar	*message)
{
	PError *ret;

	if ((ret = p_error_new ()) == NULL)
		return NULL;

	ret->code = code;
	ret->message = p_strdup (message);

	return ret;
}

P_LIB_API const pchar *
p_error_get_message (PError *error)
{
	if (error == NULL)
		return NULL;

	return error->message;
}

P_LIB_API pint
p_error_get_code (PError *error)
{
	if (error == NULL)
		return 0;

	return error->code;
}

P_LIB_API PError *
p_error_copy (PError *error)
{
	PError *ret;

	if (error == NULL)
		return NULL;

	if ((ret = p_error_new_literal (error->code, error->message)) == NULL)
		return NULL;

	return ret;
}

P_LIB_API void
p_error_set_error (PError	*error,
		   pint		code,
		   const pchar	*message)
{
	if (error == NULL)
		return;

	if (error->message != NULL)
		p_free (error->message);

	error->code = code;
	error->message = p_strdup (message);
}

P_LIB_API void
p_error_clear (PError *error)
{
	if (error == NULL)
		return;

	if (error->message != NULL)
		p_free (error->message);

	error->message = NULL;
	error->code = 0;
}

P_LIB_API void
p_error_free (PError	*error)
{
	if (error == NULL)
		return;

	if (error->message != NULL)
		p_free (error->message);

	p_free (error);
}
