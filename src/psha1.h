/* 
 * 24.10.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSHA1_H__
#define __PSHA1_H__

#include <ptypes.h>
#include <pmacros.h>

typedef struct _PHashSHA1 PHashSHA1;

P_BEGIN_DECLS

P_LIB_API PHashSHA1 *	p_sha1_new	(void);
P_LIB_API void		p_sha1_update	(PHashSHA1 *ctx, const puchar *data, pint len);
P_LIB_API void		p_sha1_finish	(PHashSHA1 *ctx, puchar output[20]);
P_LIB_API void		p_sha1_reset	(PHashSHA1 *ctx);
P_LIB_API void		p_sha1_free	(PHashSHA1 *ctx);

P_END_DECLS

#endif /* __PSHA1_H__ */

