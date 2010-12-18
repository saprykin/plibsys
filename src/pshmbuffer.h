/* 
 * 25.08.2010
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

#ifndef __PSHMBUFFER_H__
#define __PSHMBUFFER_H__

#include <ptypes.h>
#include <pmacros.h>

#include <stdlib.h>

typedef struct _PShmBuffer PShmBuffer;

P_BEGIN_DECLS

P_LIB_API PShmBuffer *	p_shm_buffer_new		(const pchar *name, psize size);
P_LIB_API void		p_shm_buffer_free		(PShmBuffer *buf);
P_LIB_API pint		p_shm_buffer_read		(PShmBuffer *buf, ppointer storage, psize len);
P_LIB_API pint		p_shm_buffer_write		(PShmBuffer *buf, ppointer data, psize len);
P_LIB_API psize		p_shm_buffer_get_free_space	(PShmBuffer *buf);
P_LIB_API pssize	p_shm_buffer_get_used_space	(PShmBuffer *buf);
P_LIB_API void		p_shm_buffer_clear		(PShmBuffer *buf);

P_END_DECLS

#endif /* __PSHMBUFFER_H__ */

